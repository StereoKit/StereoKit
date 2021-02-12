using StereoKit;
using System;
using System.Collections.Generic;
using System.Numerics;

#if WINDOWS_UWP
using Windows.Foundation;
using Windows.Perception.Spatial;
using Windows.Perception.Spatial.Surfaces;
using Windows.Storage.Streams;
#endif

namespace StereoKit.Framework
{
	/// <summary>This Requires the Spatial Perception permission in order to
	/// work properly! This contains a hack that makes it a placeholder
	/// solution only. The mesh will ONLY line up if the user remains
	/// entirely motionless during startup.</summary>
	public class OcclusionMesh : IStepper
	{
		#region Properties and Fields
		class SurfaceMesh
		{
			public Mesh           mesh;
			public DateTimeOffset lastUpdate;
			public Matrix         transform;
			public Matrix         localTransform;
			public Guid           id;
		}

		#if WINDOWS_UWP
		SpatialStationaryFrameOfReference frame = null;
		SpatialSurfaceObserver            observer;
		SpatialAnchor                     anchor;
		List<SpatialSurfaceMesh>          queuedUpdates = new List<SpatialSurfaceMesh>();
		#endif

		List<SurfaceMesh> meshes = new List<SurfaceMesh>();
		Matrix            root;
		Material          surfaceMaterial;
		double            trisPerMeter;
		Vec3              center;
		float             radius = 20;
		bool              simulation;

		public Material   Material => surfaceMaterial;
		public bool       Enabled { get; set; } = true;

		public static bool Capable { get { 
			#if WINDOWS_UWP
			return SpatialSurfaceObserver.IsSupported();
			#else
			return false;
			#endif
		} }
		#endregion

		#region Construction
		public static OcclusionMesh FromSimulation(Mesh fromMesh)
			=> FromSimulation(new Model(fromMesh, Default.Material));
		public static OcclusionMesh FromSimulation(Model fromModel)
		{
			OcclusionMesh result = new OcclusionMesh();
			result.simulation = true;

			for (int i = 0; i < fromModel.SubsetCount; i++)
			{
				result.meshes.Add(new SurfaceMesh { 
					mesh = fromModel.GetMesh     (i),
					transform   = fromModel.GetTransform(i),
				});
			}

			return result;
		}

		public OcclusionMesh(double trianglesPerMeterCubed = 2000)
		{
			trisPerMeter    = trianglesPerMeterCubed;
			surfaceMaterial = new Material(Default.ShaderUnlit);
			surfaceMaterial[MatParamName.ColorTint] = Color.BlackTransparent;
		}
		#endregion

		#region Public Methods

		public bool Initialize()
		{
			#if WINDOWS_UWP
			if (!simulation)
				return InitializeUWP();
			#endif
			return true;
		}

		public void Shutdown()
		{
		}

		public void Step()
		{
			#if WINDOWS_UWP
			StepUWP();
			#endif

			// Draw all surfaces
			for (int i = 0; i < meshes.Count; i++)
				meshes[i].mesh.Draw(surfaceMaterial, meshes[i].transform);
		}

		public void UpdateBounds(Vec3 center, float radius)
		{
			this.center = center;
			this.radius = radius;

			#if WINDOWS_UWP
			UpdateBoundsUWP();
			#endif
		}

		#endregion

		#region UWP Code
		#if WINDOWS_UWP
		private bool InitializeUWP()
		{
			// Ask or check for Spatial permissions
			SpatialSurfaceObserver.RequestAccessAsync().Completed = (i,s) => {
				if (s == AsyncStatus.Completed && i.GetResults() == SpatialPerceptionAccessStatus.Allowed) { 
					observer = new SpatialSurfaceObserver();
					observer.ObservedSurfacesChanged += OnSurfaceUpdate;
					UpdateBounds(center, radius);
					Log.Info("Got Observer");
				}
			};

			// Establish the coordinate frame of reference
			var locator = SpatialLocator.GetDefault();
			if (locator != null) {
				locator.LocatabilityChanged += OnLocated;
				OnLocated(locator, null);
				Log.Info("Located");
			}
			return true;
		}

		private void OnLocated(SpatialLocator loc, object args)
		{
			if (frame != null || loc.Locatability != SpatialLocatability.PositionalTrackingActive) return;
			frame  = loc.CreateStationaryFrameOfReferenceAtCurrentLocation();
			anchor = SpatialAnchor.TryCreateRelativeTo(frame.CoordinateSystem);
			root   = World.FromPerceptionAnchor(anchor).ToMatrix();
			Log.Info(""+World.FromPerceptionAnchor(anchor));
			UpdateBounds(center, radius);
		}

		private void StepUWP()
		{
			// Mesh updates need to be on the main thread. There are plans to
			// make this unnecessary in the future.
			for (int i = 0; i < queuedUpdates.Count; i++)
			{
				SpatialSurfaceMesh s    = queuedUpdates[i];
				SurfaceMesh        mesh = meshes.Find(m => m.id == s.SurfaceInfo.Id);
				UpdateMesh(ref mesh.mesh, s);

				mesh.localTransform = s.CoordinateSystem.TryGetTransformTo(frame.CoordinateSystem) ?? Matrix.Identity;
				mesh.transform      = mesh.localTransform * root;
				Log.Info(mesh.transform.Transform(Vec3.Zero).ToString());
			}
			queuedUpdates.Clear();
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
			byte[] data = new byte[ src.VertexPositions.Data.Length];
			DataReader.FromBuffer(src.VertexPositions.Data).ReadBytes(data);
			Vertex[] verts = new Vertex[data.Length / (sizeof(float)*4)];
			int b = 0;
			for (int i = 0; i < verts.Length; i++) {
				float x = BitConverter.ToSingle(data, b); b += sizeof(float);
				float y = BitConverter.ToSingle(data, b); b += sizeof(float);
				float z = BitConverter.ToSingle(data, b); b += sizeof(float);
				b += sizeof(float);
				verts[i] = new Vertex{ 
					pos = new Vector3(x,y,z) * src.VertexPositionScale,
					col = Color.White };
			}

			// Extract indices from the mesh buffer
			data = new byte[src.TriangleIndices.Data.Length];
			DataReader.FromBuffer(src.TriangleIndices.Data).ReadBytes(data);
			uint[] inds = new uint[data.Length/sizeof(uint)];
			for (int i = 0; i < inds.Length; i+=3)
			{
				inds[i+2] = BitConverter.ToUInt32(data, (i  ) * sizeof(uint));
				inds[i+1] = BitConverter.ToUInt32(data, (i+1) * sizeof(uint));
				inds[i  ] = BitConverter.ToUInt32(data, (i+2) * sizeof(uint));
			}

			// Update the StereoKit mesh
			mesh.SetVerts(verts);
			mesh.SetInds (inds);
		}

		void UpdateBoundsUWP() {
			if (frame != null && observer != null) { 
				SpatialBoundingVolume bounds = SpatialBoundingVolume.FromSphere(
					frame.CoordinateSystem,
					new SpatialBoundingSphere { Center = new Vector3(center.x, center.y, center.z), Radius = radius });
				observer.SetBoundingVolume(bounds);
			}
		}

		#endif
		#endregion
	}
}