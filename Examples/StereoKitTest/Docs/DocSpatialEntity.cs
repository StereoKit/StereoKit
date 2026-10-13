// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

using StereoKit;
using System;
using System.Collections.Generic;

class DocSpatialEntity : ITest
{
	SpatialCapability prevRequested;

	/// :CodeSample: SpatialEntity Spatial Spatial.Request Spatial.Disable SpatialEntity.With SpatialEntity.TryGetBounds2D SpatialCapability.PlaneTracking
	/// ### Drawing detected planes
	/// Request plane tracking once, and the device's walls, floors and
	/// tables show up in the entity list as it finds them.
	public void StartPlanes()
	{
		Spatial.Request(SpatialCapability.PlaneTracking);
	}

	public void StopPlanes()
	{
		Spatial.Disable(SpatialCapability.PlaneTracking);
	}

	public void DrawPlanes()
	{
		foreach (SpatialEntity plane in SpatialEntity.With(SpatialComponent.Bounds2D))
		{
			// Quads face Forward, same as the plane's center pose
			plane.TryGetBounds2D(out Pose center, out Vec2 size);
			Mesh.Quad.Draw(Material.Default, center.ToMatrix(new Vec3(size.x, size.y, 1)));
		}
	}
	/// :End:

	/// :CodeSample: SpatialEntity.TryGetLabel SpatialEntity.TryGetPlaneAlign SpatialLabel PlaneAlign
	/// ### Finding a table to put things on
	/// Not every device labels its planes, so this falls back to any
	/// surface that faces upward.
	static bool FindTable(out Pose surface)
	{
		foreach (SpatialEntity plane in SpatialEntity.With(SpatialComponent.Bounds2D))
		{
			bool isTable = plane.TryGetLabel(out SpatialLabel label)
				? label == SpatialLabel.Table
				: plane.TryGetPlaneAlign(out PlaneAlign align) && align == PlaneAlign.HorizontalUp;
			if (isTable && plane.TryGetBounds2D(out surface, out _))
				return true;
		}
		surface = Pose.Identity;
		return false;
	}
	/// :End:

	/// :CodeSample: SpatialEntity.CreateAnchor SpatialEntity.FindAnchor SpatialCapability.Anchor
	/// ### Anchors that survive an app restart
	/// Give an anchor a name, and `FindAnchor` gets it back in a later
	/// session. Placing it again under the same name replaces the old one.
	SpatialEntity anchor;

	public void LoadAnchor()
	{
		Spatial.Request(SpatialCapability.Anchor);
		anchor = SpatialEntity.FindAnchor("my_anchor");
	}

	// Call this from a button, a pinch, or wherever the user places content
	public void PlaceAnchor(Pose pose)
		=> anchor = SpatialEntity.CreateAnchor(pose, "my_anchor");

	public void StepAnchor()
	{
		if (anchor.Tracked.IsActive())
			Mesh.Cube.Draw(Material.Default, anchor.Pose.ToMatrix(0.1f));
	}
	/// :End:

	/// :CodeSample: SpatialEntity.Persist SpatialEntity.TryGetGuid SpatialEntity.FindAnchor SpatialEntity.Status
	/// ### Keeping track of anchors by Guid
	/// For apps that keep anchors in their own save data instead of by
	/// name. The Guid arrives a moment after `Persist`, so watch for it.
	List<Guid> savedAnchors = new List<Guid>();

	public void StepSavedAnchors()
	{
		foreach (SpatialEntity anchor in SpatialEntity.With(SpatialComponent.Anchor))
		{
			if (anchor.HasChanged(SpatialComponent.Persistence) && anchor.TryGetGuid(out Guid guid))
				savedAnchors.Add(guid);
		}

		// Storage no longer having a Guid shows up as Failed
		foreach (SpatialEntity anchor in SpatialEntity.Removed)
		{
			if (anchor.Status == SpatialStatus.Failed && anchor.TryGetGuid(out Guid lost))
				savedAnchors.Remove(lost);
		}
	}

	// In a later session, each Guid loads back into its anchor
	public List<SpatialEntity> RestoreAnchors()
	{
		List<SpatialEntity> result = new List<SpatialEntity>();
		foreach (Guid guid in savedAnchors)
			result.Add(SpatialEntity.FindAnchor(guid));
		return result;
	}
	/// :End:

	/// :CodeSample: SpatialEntity.Destroy SpatialEntity.Unpersist
	/// ### Removing anchors
	static void RemoveAnchors(SpatialEntity anchor, Guid savedGuid)
	{
		// Gone for good, from this session, storage, and its name
		anchor.Destroy();

		// Stays for this session, but won't come back next launch
		anchor.Unpersist();

		// Cleans up a Guid from an earlier session, without loading it
		SpatialEntity.Unpersist(savedGuid);
	}
	/// :End:

	/// :CodeSample: SpatialEntity.MarkerText SpatialEntity.Pose SpatialEntity.Tracked SpatialEntity.New SpatialEntity.NewWith SpatialCapability.QRCode
	/// ### Reading QR codes
	public void StartQRCodes()
	{
		Spatial.Request(SpatialCapability.QRCode);
	}

	public void StepQRCodes()
	{
		// New lists each entity exactly once, on the frame it appears
		foreach (SpatialEntity code in SpatialEntity.NewWith(SpatialComponent.Marker))
			Log.Info($"Found a QR code: {code.MarkerText}");

		foreach (SpatialEntity code in SpatialEntity.With(SpatialComponent.Marker))
		{
			// Codes keep their last pose out of view, so only show ones in sight
			if (code.Tracked.IsActive())
				Text.Add(code.MarkerText ?? "", code.Pose.ToMatrix());
		}
	}
	/// :End:

	/// :CodeSample: SpatialEntity.TryGetMarker Spatial.ArUcoDictionary Spatial.AprilTagDictionary Spatial.SetMarkerSize Spatial.SetMarkerStationary ArUcoDict SpatialCapability.ArUco
	/// ### Finding an ArUco marker by id
	/// The dictionary must match the markers you printed, or they won't be
	/// detected. AprilTags work the same way, via `AprilTagDictionary`.
	public void StartArUco()
	{
		Spatial.ArUcoDictionary = ArUcoDict.Dict4x4_50;
		// A 10cm printed marker, used by runtimes that support known sizes
		Spatial.SetMarkerSize(MarkerType.ArUco, 0.1f);
		// Markers taped to a wall can have their pose refined over time
		Spatial.SetMarkerStationary(MarkerType.ArUco, true);
		Spatial.Request(SpatialCapability.ArUco);
	}

	static bool FindMarker(uint id, out Pose pose)
	{
		foreach (SpatialEntity marker in SpatialEntity.With(SpatialComponent.Marker))
		{
			if (marker.TryGetMarker(out MarkerType type, out uint markerId) && type == MarkerType.ArUco && markerId == id)
				return marker.TryGetBounds2D(out pose, out _);
		}
		pose = Pose.Identity;
		return false;
	}
	/// :End:

	/// :CodeSample: Spatial.IsSupported Spatial.IsRequested Spatial.IsRunning Spatial.Capabilities Spatial.ComponentsFor Spatial.Running Spatial.Requested
	/// ### Checking what the device supports
	/// Supported is what the device can do, Requested is what's been asked
	/// for, and Running is what's started up and working right now.
	static bool CanFindTables()
		=> Spatial.IsSupported(SpatialCapability.PlaneTracking, SpatialComponent.Label);

	static string PlaneStatus()
	{
		if (Spatial.IsRunning  (SpatialCapability.PlaneTracking)) return "Finding planes";
		if (Spatial.IsRequested(SpatialCapability.PlaneTracking)) return "Starting up...";
		return "Plane tracking is off";
	}
	/// :End:

	/// :CodeSample: SpatialEntity.TryGetMesh2D SpatialEntity.HasChanged SpatialEntity.Changed SpatialEntity.Removed
	/// ### Keeping a mesh for each plane
	/// Filling a Mesh isn't free, so this only refills when the plane's
	/// shape changes, and drops the mesh when the plane goes away.
	Dictionary<SpatialEntity, Mesh> planeMeshes = new Dictionary<SpatialEntity, Mesh>();

	public void StepPlaneMeshes()
	{
		foreach (SpatialEntity plane in SpatialEntity.With(SpatialComponent.Mesh2D))
		{
			bool isNew = !planeMeshes.TryGetValue(plane, out Mesh mesh);
			if (isNew)
			{
				mesh = new Mesh();
				planeMeshes.Add(plane, mesh);
			}

			// A null mesh only fetches the origin, which moves every frame
			bool refill = isNew || plane.HasChanged(SpatialComponent.Mesh2D);
			plane.TryGetMesh2D(refill ? mesh : null, out Pose origin);
			mesh.Draw(Material.Default, origin.ToMatrix());
		}

		// Removed entities are still readable this frame, so clean up here
		foreach (SpatialEntity plane in SpatialEntity.Removed)
			planeMeshes.Remove(plane);
	}
	/// :End:

	/// :CodeSample: SpatialEntity.TryGetPolygon
	/// ### Outlining a plane's real shape
	/// Bounds are a rectangle, while the polygon follows the actual edge,
	/// like an L-shaped counter. This runs every frame, so it reuses one
	/// array instead of allocating a new one each time.
	static Vec2[] outline;

	static void DrawOutline(SpatialEntity plane)
	{
		if (!plane.TryGetPolygon(out Pose origin, ref outline, out int count)) return;

		Matrix toWorld = origin.ToMatrix();
		for (int i = 0; i < count; i++)
		{
			Vec3 start = toWorld.Transform(outline[i].XY0);
			Vec3 end   = toWorld.Transform(outline[(i + 1) % count].XY0);
			Lines.Add(start, end, Color32.White, 0.005f);
		}
	}
	/// :End:

	public void Initialize()
	{
		prevRequested = Spatial.Requested;
		StartPlanes();
		StartQRCodes();
		StartArUco();
		LoadAnchor();
	}

	public void Step()
	{
		DrawPlanes();
		StepAnchor();
		StepSavedAnchors();
		StepQRCodes();
		StepPlaneMeshes();
		foreach (SpatialEntity plane in SpatialEntity.With(SpatialComponent.Polygon))
			DrawOutline(plane);

		if (FindTable(out Pose table))   Mesh.Sphere.Draw(Material.Default, table.ToMatrix(0.05f));
		if (FindMarker(0, out Pose mrk)) Mesh.Sphere.Draw(Material.Default, mrk  .ToMatrix(0.05f));
		if (CanFindTables())             Text.Add(PlaneStatus(), Matrix.T(0, 0.2f, -0.5f));
	}

	public void Shutdown()
	{
		// Only turn off what this doc turned on, so later tests start clean
		Spatial.Disable(Spatial.Capabilities & ~prevRequested);
		planeMeshes.Clear();
	}
}
