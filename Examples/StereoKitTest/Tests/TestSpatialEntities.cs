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
			markerSizesCm[i] = SpatialEntity.GetMarkerSize(markerTypes[i]) * 100;

		Tests.Test(TestInvalidEntity);
		Tests.Test(TestMarkerSettings);
		Tests.Test(TestEmptyPersistId);
		if ((SpatialEntity.Capabilities & SpatialCapability.Anchor) == 0)
			Tests.Test(TestNoAnchorSupport);

		prevEnabled = SpatialEntity.Enabled;
		SpatialEntity.Enable(SpatialEntity.Capabilities);
	}

	public void Shutdown()
	{
		// Disabling everything would also turn off the Anchor system for later tests
		SpatialEntity.Disable(SpatialEntity.Capabilities & ~prevEnabled);
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
			&& none.PersistId  == Guid.Empty
			&& none.Components == SpatialComponent.None
			&& !none.TryGetAnchor(out _)
			&& !none.Persist()
			&& !none.Destroy();
	}

	bool TestMarkerSettings()
	{
		float        prevSize       = SpatialEntity.GetMarkerSize(MarkerType.QrCode);
		bool         prevStationary = SpatialEntity.GetMarkerStationary(MarkerType.Aruco);
		ArucoDict    prevAruco      = SpatialEntity.ArucoDictionary;
		AprilTagDict prevApril      = SpatialEntity.AprilTagDictionary;

		SpatialEntity.SetMarkerSize      (MarkerType.QrCode, 0.05f);
		SpatialEntity.SetMarkerStationary(MarkerType.Aruco,  true);
		SpatialEntity.ArucoDictionary    = ArucoDict.Dict5x5_100;
		SpatialEntity.AprilTagDictionary = AprilTagDict.Tag16h5;
		bool result =
			SpatialEntity.GetMarkerSize      (MarkerType.QrCode)  == 0.05f &&
			SpatialEntity.GetMarkerSize      (MarkerType.MicroQr) == 0     && // Settings are per type
			SpatialEntity.GetMarkerStationary(MarkerType.Aruco)            &&
			!SpatialEntity.GetMarkerStationary(MarkerType.AprilTag)        &&
			SpatialEntity.ArucoDictionary    == ArucoDict.Dict5x5_100      &&
			SpatialEntity.AprilTagDictionary == AprilTagDict.Tag16h5;

		SpatialEntity.SetMarkerSize(MarkerType.QrCode, -1);
		result &= SpatialEntity.GetMarkerSize(MarkerType.QrCode) == 0; // Negative sizes mean unknown

		SpatialEntity.SetMarkerSize      (MarkerType.QrCode, prevSize);
		SpatialEntity.SetMarkerStationary(MarkerType.Aruco,  prevStationary);
		SpatialEntity.ArucoDictionary    = prevAruco;
		SpatialEntity.AprilTagDictionary = prevApril;
		return result;
	}

	bool TestEmptyPersistId()
		=> !SpatialEntity.FromPersistId(Guid.Empty).Valid
		&& !SpatialEntity.Unpersist(Guid.Empty);

	// Without anchor support, requests fail up front rather than waiting.
	bool TestNoAnchorSupport()
		=> !SpatialEntity.CreateAnchor(Pose.Identity).Valid
		&& !SpatialEntity.FromPersistId(Guid.NewGuid()).Valid;

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
				SpatialEntity.FromPersistId(id);
		UI.PopEnabled();
		UI.SameLine();
		UI.PushEnabled((SpatialEntity.Enabled & SpatialCapability.Anchor) > 0);
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
			if ((entity.Changed & SpatialComponent.Persistence) > 0)
				SavedAnchorIds.Add(entity.PersistId);
			DrawStatus(entity);
			if (showMesh2d) DrawMesh2D(entity);
		}
		// Persisted entities only leave once storage no longer has them
		foreach (SpatialEntity entity in SpatialEntity.Removed)
		{
			if (entity.Status == SpatialStatus.Failed)
				Log.Info($"Spatial entity {entity.PersistId} failed");
			SavedAnchorIds.Remove(entity.PersistId);
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
			Guid id = entity.PersistId;
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
				SpatialEntity.SetMarkerSize(type, markerSizesCm[i] / 100);
			UI.SameLine();
			bool stationary = SpatialEntity.GetMarkerStationary(type);
			if (UI.Toggle("Static", ref stationary))
				SpatialEntity.SetMarkerStationary(type, stationary);
			UI.PopId();
		}
		if (UI.Button($"ArUco {SpatialEntity.ArucoDictionary}"))
			SpatialEntity.ArucoDictionary = (ArucoDict)(((int)SpatialEntity.ArucoDictionary + 1) % ((int)ArucoDict.Dict7x7_1000 + 1));
		UI.SameLine();
		if (UI.Button($"AprilTag {SpatialEntity.AprilTagDictionary}"))
			SpatialEntity.AprilTagDictionary = (AprilTagDict)(((int)SpatialEntity.AprilTagDictionary + 1) % ((int)AprilTagDict.Tag36h11 + 1));
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
