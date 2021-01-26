#if WINDOWS_UWP

using StereoKit;
using System;
using System.Collections.Generic;
using System.Numerics;
using Windows.Foundation;
using Windows.Perception.Spatial;
using Windows.Perception.Spatial.Surfaces;
using Windows.Storage.Streams;

namespace StereoKitTest
{
	class DemoPerceptionAnchor : ITest
	{
		OcclusionMesh occlusion;
		Pose windowPose        = new Pose(Vec3.Forward, Quat.LookDir(-Vec3.Forward));
		bool settingsWireframe = true;
		int  settingsColor     = 0;

		public void Initialize()
		{
			OcclusionMesh occlusion = new OcclusionMesh(Color.HSV(0,1,1));
			occlusion.Material.Wireframe = true;
		}
		public void Shutdown()
		{
			occlusion = null;
		}

		public void Update()
		{
			occlusion.Step();

			UI.WindowBegin("Settings", ref windowPose, Vec2.Zero);
			if (UI.Toggle("Wireframe", ref settingsWireframe))
				occlusion.Material.Wireframe = settingsWireframe;
			if (UI.Radio("Red",   settingsColor == 0)) { settingsColor = 0; occlusion.Material.SetColor("color", Color.HSV(0,1,1)); } UI.SameLine();
			if (UI.Radio("White", settingsColor == 1)) { settingsColor = 1; occlusion.Material.SetColor("color", Color.White); } UI.SameLine();
			if (UI.Radio("Black", settingsColor == 2)) { settingsColor = 2; occlusion.Material.SetColor("color", Color.BlackTransparent); }
			UI.WindowEnd();
		}
	}

	/// <summary>This Requires the Spatial Perception permission in order to
	/// work properly! This contains a hack that makes it a placeholder solution
	/// only. The mesh will ONLY line up if the user remains entirely motionless
	/// during startup.</summary>
	public class OcclusionMesh
	{
		class SurfaceMesh
		{
			public Mesh           mesh;
			public DateTimeOffset lastUpdate;
			public Matrix4x4      at;
			public Guid           id;
		}

		SpatialStationaryFrameOfReference frame = null;
		SpatialSurfaceObserver            observer;
		SpatialAnchor                     anchor;
		List<SurfaceMesh>         meshes        = new List<SurfaceMesh>();
		List<SpatialSurfaceMesh>  queuedUpdates = new List<SpatialSurfaceMesh>();
		Matrix4x4         root;
		Material          surfaceMaterial;
		double            trisPerMeter;
		Vector3           center;
		float             radius = 20;

		public Material Material => surfaceMaterial;

		public OcclusionMesh(Color color, double trianglesPerMeterCubed = 2000)
		{
			// Ask or check for Spatial permissions
			SpatialSurfaceObserver.RequestAccessAsync().Completed = (i,s) => {
				if (s == AsyncStatus.Completed && i.GetResults() == SpatialPerceptionAccessStatus.Allowed) { 
					observer = new SpatialSurfaceObserver();
					observer.ObservedSurfacesChanged += OnSurfaceUpdate;
					UpdateBounds(center, radius);
				}
			};

			// Establish the coordinate frame of reference
			var locator = SpatialLocator.GetDefault();
			if (locator != null) {
				locator.LocatabilityChanged += OnLocated;
				OnLocated(locator, null);
			}

			trisPerMeter    = trianglesPerMeterCubed;
			surfaceMaterial = new Material(Default.ShaderUnlit);
			surfaceMaterial[MatParamName.ColorTint] = color;
		}

		private void OnLocated(SpatialLocator loc, object args)
		{
			if (frame != null || loc.Locatability != SpatialLocatability.PositionalTrackingActive) return;
			frame  = loc.CreateStationaryFrameOfReferenceAtCurrentLocation();
			anchor = SpatialAnchor.TryCreateRelativeTo(frame.CoordinateSystem);
			root   = World.FromPerceptionAnchor(anchor).ToMatrix();
			UpdateBounds(center, radius);
		}

		public void UpdateBounds(Vector3 center, float radius)
		{
			this.center = center;
			this.radius = radius;

			if (frame != null && observer != null) { 
				SpatialBoundingVolume bounds = SpatialBoundingVolume.FromSphere(
					frame.CoordinateSystem,
					new SpatialBoundingSphere { Center = new Vector3(center.X, center.Y, center.Z), Radius = radius });
				observer.SetBoundingVolume(bounds);
			}
		}

		private void OnSurfaceUpdate(SpatialSurfaceObserver observer, object args)
		{
			var surfaceDict = observer.GetObservedSurfaces();
			foreach (var surface in surfaceDict)
			{
				// Find or add a surface mesh for this surface
				SurfaceMesh mesh = meshes.Find(m=>m.id == surface.Key);
				if (mesh == null)
				{
					mesh = new SurfaceMesh {
						mesh       = new Mesh(),
						id         = surface.Key,
						lastUpdate = new DateTimeOffset() };
					meshes.Add(mesh);
				}

				// Ask for an update to the mesh data, if it's old
				if (surface.Value.UpdateTime > mesh.lastUpdate)
				{
					mesh.lastUpdate = surface.Value.UpdateTime;
					SpatialSurfaceMeshOptions opts = new SpatialSurfaceMeshOptions();
					opts.IncludeVertexNormals = false;
					opts.TriangleIndexFormat  = Windows.Graphics.DirectX.DirectXPixelFormat.R32UInt;
					opts.VertexPositionFormat = Windows.Graphics.DirectX.DirectXPixelFormat.R32G32B32A32Float;
					surface.Value.TryComputeLatestMeshAsync(trisPerMeter, opts).Completed = (info, state) => {
						// Send update to the main thread for upload to GPU
						if (state == Windows.Foundation.AsyncStatus.Completed)
							queuedUpdates.Add(info.GetResults());
					};
				}
			}
		}

		void UpdateMesh(ref Mesh mesh, SpatialSurfaceMesh src)
		{
			// Extract vertex positions from the mesh buffer
			byte[] data = new byte[ src.VertexPositions.Data.Length ];
			DataReader.FromBuffer(src.VertexPositions.Data).ReadBytes(data);
			Vertex[] verts = new Vertex[data.Length / (sizeof(float)*4)];
			int b = 0;
			for (int i = 0; i < verts.Length; i++) {
				float x = BitConverter.ToSingle(data, b); b += sizeof(float);
				float y = BitConverter.ToSingle(data, b); b += sizeof(float);
				float z = BitConverter.ToSingle(data, b); b += sizeof(float);
				b += sizeof(float);
				verts[i] = new Vertex{ pos = new Vector3(x,y,z) };
			}

			// Extract indices from the mesh buffer
			data = new byte[src.TriangleIndices.Data.Length];
			DataReader.FromBuffer(src.TriangleIndices.Data).ReadBytes(data);
			uint[] inds = new uint[data.Length/sizeof(uint)];
			for (int i = 0; i < inds.Length; i++)
			{
				inds[i] = BitConverter.ToUInt32(data, i * sizeof(uint));
			}

			// Update the StereoKit mesh
			mesh.SetVerts(verts);
			mesh.SetInds (inds);
		}

		public void Step()
		{
			// Mesh updates need to be on the main thread. There are plans to
			// make this unnecessary in the future.
			for (int i = 0; i < queuedUpdates.Count; i++)
			{
				SpatialSurfaceMesh s    = queuedUpdates[i];
				SurfaceMesh        mesh = meshes.Find(m => m.id == s.SurfaceInfo.Id);
				UpdateMesh(ref mesh.mesh, s);

				mesh.at = root * (s.CoordinateSystem.TryGetTransformTo(frame.CoordinateSystem) ?? Matrix4x4.Identity);
			}
			queuedUpdates.Clear();

			// Draw all surfaces
			for (int i = 0; i < meshes.Count; i++)
				meshes[i].mesh.Draw(surfaceMaterial, meshes[i].at);
		}
	}
}

#endif