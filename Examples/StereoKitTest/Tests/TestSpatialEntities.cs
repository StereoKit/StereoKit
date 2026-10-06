// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

using System;
using System.Collections.Generic;
using StereoKit;

// Exercises the parts of SpatialEntity that DemoSpatialEntity doesn't:
// restoring persisted anchors by id, per-anchor persist/unpersist/destroy
// with Status, mesh2d against the polygon outline, parented anchors, and
// the marker settings. The headless checks only cover what works without
// spatial support, the rest needs a device.
class TestSpatialEntities : ITest
{
	Pose     windowPose = (Demo.contentPose * Matrix.T(-0.2f, 0, 0)).Pose;
	Material mesh2dMat;
	bool     showMesh2d = true;
	SpatialCapability prevEnabled;
	Dictionary<SpatialEntity, Mesh> mesh2ds = new Dictionary<SpatialEntity, Mesh>();

	static readonly MarkerType[] markerTypes = { MarkerType.QrCode, MarkerType.MicroQr, MarkerType.Aruco, MarkerType.AprilTag };
	// Size changes restart marker tracking, so sliders only apply on release
	float[] markerSizesCm = new float[markerTypes.Length];

	public void Initialize()
	{
		mesh2dMat = Material.Default.Copy();
		mesh2dMat.Wireframe = true;
		mesh2dMat[MatParamName.ColorTint] = new Color(1, 0.5f, 0);
		for (int i = 0; i < markerTypes.Length; i++)
			markerSizesCm[i] = Spatial.GetMarkerSize(markerTypes[i]) * 100;

		Tests.Test(TestInvalidEntity);
		Tests.Test(TestMarkerSettings);
		Tests.Test(TestEmptyGuid);
		if ((Spatial.Capabilities & SpatialCapability.Anchor) == 0)
			Tests.Test(TestNoAnchorSupport);

		prevEnabled = Spatial.Enabled;
		Spatial.Enable(Spatial.Capabilities);
	}

	public void Shutdown()
	{
		// Disabling everything would also turn off the Anchor system for later tests
		Spatial.Disable(Spatial.Capabilities & ~prevEnabled);
		mesh2ds.Clear();
	}

	///////////////////////////////////////////
	// Headless checks                       //
	///////////////////////////////////////////

	bool TestInvalidEntity()
	{
		SpatialEntity none = default;
		return !none.Valid
			&& none.Status     == SpatialStatus.None
			&& !none.TryGetGuid(out _)
			&& none.Components == SpatialComponent.None
			&& !none.TryGetAnchor(out _)
			&& !none.Persist()
			&& !none.Destroy();
	}

	bool TestMarkerSettings()
	{
		float        prevSize       = Spatial.GetMarkerSize(MarkerType.QrCode);
		bool         prevStationary = Spatial.GetMarkerStationary(MarkerType.Aruco);
		ArucoDict    prevAruco      = Spatial.ArucoDictionary;
		AprilTagDict prevApril      = Spatial.AprilTagDictionary;

		Spatial.SetMarkerSize      (MarkerType.QrCode, 0.05f);
		Spatial.SetMarkerStationary(MarkerType.Aruco,  true);
		Spatial.ArucoDictionary    = ArucoDict.Dict5x5_100;
		Spatial.AprilTagDictionary = AprilTagDict.Tag16h5;
		bool result =
			Spatial.GetMarkerSize      (MarkerType.QrCode)  == 0.05f &&
			Spatial.GetMarkerSize      (MarkerType.MicroQr) == 0     && // Settings are per type
			Spatial.GetMarkerStationary(MarkerType.Aruco)            &&
			!Spatial.GetMarkerStationary(MarkerType.AprilTag)        &&
			Spatial.ArucoDictionary    == ArucoDict.Dict5x5_100      &&
			Spatial.AprilTagDictionary == AprilTagDict.Tag16h5;

		Spatial.SetMarkerSize(MarkerType.QrCode, -1);
		result &= Spatial.GetMarkerSize(MarkerType.QrCode) == 0; // Negative sizes mean unknown

		Spatial.SetMarkerSize      (MarkerType.QrCode, prevSize);
		Spatial.SetMarkerStationary(MarkerType.Aruco,  prevStationary);
		Spatial.ArucoDictionary    = prevAruco;
		Spatial.AprilTagDictionary = prevApril;
		return result;
	}

	bool TestEmptyGuid()
		=> !SpatialEntity.Find(Guid.Empty).Valid
		&& !SpatialEntity.Unpersist(Guid.Empty);

	// Without anchor support, requests fail up front rather than waiting.
	bool TestNoAnchorSupport()
		=> !SpatialEntity.CreateAnchor(Pose.Identity).Valid
		&& !SpatialEntity.Find(Guid.NewGuid()).Valid;

	///////////////////////////////////////////
	// Interactive                           //
	///////////////////////////////////////////

	public void Step()
	{
		UI.WindowBegin("Spatial Entity Tests", ref windowPose, new Vec2(0.32f, 0));

		// Ids storage no longer has should come back Failed
		UI.PushEnabled(SavedAnchorIds.Count > 0);
		if (UI.Button($"Restore Saved ({SavedAnchorIds.Count})"))
			foreach (Guid id in SavedAnchorIds.Ids)
				SpatialEntity.Find(id);
		UI.PopEnabled();
		UI.SameLine();
		UI.PushEnabled((Spatial.Enabled & SpatialCapability.Anchor) > 0);
		if (UI.Button("Anchor on Nearest Plane"))
			AnchorOnNearestPlane();
		UI.PopEnabled();
		UI.Toggle("Show Mesh2D", ref showMesh2d);

		UI.HSeparator();
		AnchorList();

		UI.HSeparator();
		MarkerSettings();
		UI.WindowEnd();

		foreach (SpatialEntity entity in SpatialEntity.All)
		{
			if ((entity.Changed & SpatialComponent.Persistence) > 0 && entity.TryGetGuid(out Guid guid))
				SavedAnchorIds.Add(guid);
			DrawStatus(entity);
			if (showMesh2d) DrawMesh2D(entity);
		}
		// Persisted entities only leave once storage no longer has them
		foreach (SpatialEntity entity in SpatialEntity.Removed)
		{
			entity.TryGetGuid(out Guid guid);
			if (entity.Status == SpatialStatus.Failed)
				Log.Info($"Spatial entity {guid} failed");
			SavedAnchorIds.Remove(guid);
			mesh2ds.Remove(entity);
		}
	}

	// Every anchor, including pending lookups that have no pose yet, with
	// the requests that can be made of it.
	void AnchorList()
	{
		int shown = 0;
		foreach (SpatialEntity entity in SpatialEntity.All)
		{
			bool isAnchor = (entity.Components & SpatialComponent.Anchor) > 0;
			bool isLookup = entity.Status == SpatialStatus.Pending && (entity.Components & SpatialComponent.Persistence) > 0;
			if (!isAnchor && !isLookup) continue;
			if (shown++ >= 8) { UI.Label("..."); break; }

			UI.PushId(entity.GetHashCode());
			entity.TryGetGuid(out Guid id);
			UI.Label($"{(id == Guid.Empty ? "unsaved " : id.ToString().Substring(0, 8))} {entity.Status} {(entity.Tracked.IsActive() ? "tracked" : "untracked")}", new Vec2(0.13f, 0));
			UI.SameLine();
			if (UI.Button("Persist"))   entity.Persist();
			UI.SameLine();
			if (UI.Button("Unpersist")) entity.Unpersist();
			UI.SameLine();
			if (UI.Button("Destroy"))   entity.Destroy();
			UI.PopId();
		}
		if (shown == 0) UI.Label("No anchors yet");
	}

	void MarkerSettings()
	{
		for (int i = 0; i < markerTypes.Length; i++)
		{
			MarkerType type = markerTypes[i];
			UI.PushId((int)type);
			UI.Label($"{type} {markerSizesCm[i]:0}cm", new Vec2(0.09f, 0));
			UI.SameLine();
			if (UI.HSlider("size", ref markerSizesCm[i], 0, 30, 1, 0.12f, UIConfirm.Push, UINotify.Finalize))
				Spatial.SetMarkerSize(type, markerSizesCm[i] / 100);
			UI.SameLine();
			bool stationary = Spatial.GetMarkerStationary(type);
			if (UI.Toggle("Static", ref stationary))
				Spatial.SetMarkerStationary(type, stationary);
			UI.PopId();
		}
		if (UI.Button($"ArUco {Spatial.ArucoDictionary}"))
			Spatial.ArucoDictionary = (ArucoDict)(((int)Spatial.ArucoDictionary + 1) % ((int)ArucoDict.Dict7x7_1000 + 1));
		UI.SameLine();
		if (UI.Button($"AprilTag {Spatial.AprilTagDictionary}"))
			Spatial.AprilTagDictionary = (AprilTagDict)(((int)Spatial.AprilTagDictionary + 1) % ((int)AprilTagDict.Tag36h11 + 1));
	}

	// Few runtimes support parented anchors yet, so on those this shows up
	// as a Failed anchor rather than doing nothing.
	void AnchorOnNearestPlane()
	{
		Vec3          head    = Input.Head.position;
		SpatialEntity nearest = default;
		Pose          center  = Pose.Identity;
		float         best    = float.MaxValue;
		foreach (SpatialEntity entity in SpatialEntity.With(SpatialComponent.Bounds2d | SpatialComponent.PlaneAlignment))
		{
			if (!entity.TryGetBounds2D(out Pose c, out _)) continue;
			float dist = Vec3.DistanceSq(c.position, head);
			if (dist >= best) continue;
			best    = dist;
			nearest = entity;
			center  = c;
		}
		if (!nearest.Valid) { Log.Info("No plane to anchor on"); return; }
		SpatialEntity.CreateAnchor(center, parent: nearest);
	}

	void DrawStatus(SpatialEntity entity)
	{
		if (!entity.TryGetAnchor(out Pose pose)) return;
		Color color = entity.Status switch {
			SpatialStatus.Ready   => new Color(0.3f, 1, 0.3f),
			SpatialStatus.Pending => new Color(1, 1, 0.3f),
			_                     => new Color(1, 0.3f, 0.3f),
		};
		Text.Add(entity.Status.ToString(), pose.ToMatrix() * Matrix.T(0, -0.02f, 0), Text.MakeStyle(Font.Default, 0.01f, color), Pivot.TopCenter);
	}

	// A plane's mesh2d should sit exactly inside its polygon outline, which
	// checks the handedness conversion of 2D vertex data.
	void DrawMesh2D(SpatialEntity entity)
	{
		if ((entity.Components & SpatialComponent.Mesh2d) == 0) return;
		if (!mesh2ds.TryGetValue(entity, out Mesh mesh))
		{
			mesh = new Mesh();
			if (!entity.TryGetMesh2D(mesh, out _)) return;
			mesh2ds.Add(entity, mesh);
		}
		Mesh refill = (entity.Changed & SpatialComponent.Mesh2d) > 0 ? mesh : null;
		if (entity.TryGetMesh2D(refill, out Pose origin))
			mesh.Draw(mesh2dMat, origin.ToMatrix());
	}
}
