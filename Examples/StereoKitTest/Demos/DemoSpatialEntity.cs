using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using StereoKit;

class DemoSpatialEntity : ITest
{
	string title       = "Spatial Entities";
	string description = "SpatialEntity is StereoKit's interface to the device's understanding of the physical environment: planes like walls and tables, tracked markers like QR codes, and anchors the app has placed. Enable the capabilities the device supports, and poll the entity list each frame!";

	Pose     windowPose = (Demo.contentPose * Matrix.T(0.2f, 0, 0)).Pose;
	Material meshMat    = null;
	bool     showMeshes = true;
	bool     persistNew = false;

	// Per-entity cache for anything expensive to fetch or build, only
	// refreshed when the entity's Changed flags say the data is new.
	class Visual
	{
		public Mesh   mesh;
		public Pose   meshOrigin;
		public Vec2[] polygon;
		public Pose   polygonOrigin;
		public string name;
		public string anchorLabel;
		public Guid   persistId;
	}
	Dictionary<SpatialEntity, Visual> visuals = new Dictionary<SpatialEntity, Visual>();
	SpatialCapability                 prevEnabled;

	static readonly (SpatialCapability cap, string name)[] capNames = {
		(SpatialCapability.Anchor,        "Anchors"),
		(SpatialCapability.PlaneTracking, "Plane Tracking"),
		(SpatialCapability.QrCode,        "QR Codes"),
		(SpatialCapability.MicroQr,       "Micro QR Codes"),
		(SpatialCapability.Aruco,         "ArUco Markers"),
		(SpatialCapability.AprilTag,      "AprilTags"),
	};

	public bool Enabled => true;

	public void Initialize()
	{
		meshMat           = Material.Default.Copy();
		meshMat.Wireframe = true;

		prevEnabled = SpatialEntity.Enabled;
		SpatialEntity.Enable(SpatialEntity.Capabilities);
	}

	public void Shutdown()
	{
		// Disabling everything would also turn off the Anchor system for later demos
		SpatialEntity.Disable(SpatialEntity.Capabilities & ~prevEnabled);
		visuals.Clear();
	}

	public void Step()
	{
		UI.WindowBegin("Spatial Entities", ref windowPose);

		SpatialCapability supported = SpatialEntity.Capabilities;
		SpatialCapability enabled   = SpatialEntity.Enabled;
		foreach ((SpatialCapability cap, string name) in capNames)
		{
			UI.PushEnabled((supported & cap) > 0);
			bool on = (enabled & cap) > 0;
			if (UI.Toggle(name, ref on))
			{
				if (on) SpatialEntity.Enable (cap);
				else    SpatialEntity.Disable(cap);
			}
			UI.PopEnabled();
		}

		// Pending entities are ones still being created or loaded, like a
		// persisted anchor waiting for the system to find it again.
		int tracked = 0, pending = 0;
		foreach (SpatialEntity entity in SpatialEntity.All)
		{
			if (entity.Tracked.IsActive())               tracked++;
			if (entity.Status == SpatialStatus.Pending) pending++;
		}
		UI.HSeparator();
		UI.Label($"{SpatialEntity.Count()} entities, {tracked} tracked, {pending} pending");
		UI.Label($"{SpatialEntity.New.Count} new, {SpatialEntity.Removed.Count} removed this frame");
		UI.Toggle("Show Meshes", ref showMeshes);

		// Anchor creation + persistence
		UI.HSeparator();
		UI.PushEnabled((SpatialEntity.Enabled & SpatialCapability.Anchor) > 0);
		if (UI.Button("Create Anchor"))
		{
			Pose head = Input.Head;
			Vec3 at   = head.position + head.Forward * 0.5f;
			SpatialEntity.CreateAnchor(new Pose(at, Quat.LookAt(at, head.position)), persistNew);
		}
		UI.SameLine();
		UI.PushEnabled((SpatialEntity.ComponentsFor(SpatialCapability.Anchor) & SpatialComponent.Persistence) > 0);
		UI.Toggle("Persist", ref persistNew);
		UI.PopEnabled();
		UI.PopEnabled();

		UI.PushEnabled(SavedAnchorIds.Count > 0);
		if (UI.Button($"Forget Saved ({SavedAnchorIds.Count})"))
			SavedAnchorIds.Forget();
		UI.PopEnabled();
		UI.WindowEnd();

		foreach (SpatialEntity entity in SpatialEntity.All)
			DrawEntity(entity);
		// Persisted entities only leave once they're gone from storage too,
		// like a destroyed anchor, so their saved id goes with them.
		foreach (SpatialEntity entity in SpatialEntity.Removed)
		{
			if (visuals.TryGetValue(entity, out Visual vis)) TrackSavedId(vis, Guid.Empty);
			visuals.Remove(entity);
		}

		Demo.ShowSummary(title, description,
			new Bounds(V.XY0(0.2f, -0.06f), V.XYZ(.3f, .28f, 0.1f)));
	}

	void DrawEntity(SpatialEntity entity)
	{
		if (!visuals.TryGetValue(entity, out Visual vis))
		{
			vis = new Visual();
			visuals.Add(entity, vis);
		}
		SpatialComponent components = entity.Components;
		SpatialComponent changed    = entity.Changed;
		bool             tracked    = entity.Tracked.IsActive();

		entity.TryGetLabel(out SpatialLabel label);
		Color32 color = tracked ? LabelColor(label) : new Color32(128, 128, 128, 255);

		bool hasRect = entity.TryGetBounds2D(out Pose center, out Vec2 size2);
		if (hasRect)
		{
			DrawRect(center, size2, color);
			Lines.AddAxis(center, 0.05f);
		}
		if (entity.TryGetBounds3D(out Pose boxCenter, out Vec3 size3))
			DrawBox(boxCenter, size3, color);

		// Polygon points allocate a copy on fetch, so they're cached, and
		// refreshed only when Changed says there's new data
		if ((components & SpatialComponent.Polygon) > 0 && (vis.polygon == null || (changed & SpatialComponent.Polygon) > 0))
			entity.TryGetPolygon(out vis.polygonOrigin, out vis.polygon);
		if (vis.polygon != null && vis.polygon.Length > 1)
		{
			for (int i = 0; i < vis.polygon.Length; i++)
			{
				Vec3 a = vis.polygonOrigin.position + vis.polygonOrigin.orientation * vis.polygon[i].XY0;
				Vec3 b = vis.polygonOrigin.position + vis.polygonOrigin.orientation * vis.polygon[(i + 1) % vis.polygon.Length].XY0;
				Lines.Add(a, b, color, 0.003f);
			}
		}

		// Anchors draw as an axis gizmo, with their persist id when stored
		if (entity.TryGetAnchor(out Pose anchorPose))
		{
			Lines.AddAxis(anchorPose, 0.1f);
			if (vis.anchorLabel == null || (changed & SpatialComponent.Persistence) > 0)
			{
				Guid persistId = entity.PersistId;
				vis.anchorLabel = persistId == Guid.Empty ? "anchor" : persistId.ToString().Substring(0, 8);
				TrackSavedId(vis, persistId);
			}
			Text.Add(vis.anchorLabel, anchorPose.ToMatrix(), Pivot.TopCenter);
		}

		// A text label: decoded marker data, marker id, semantic label, or
		// plane alignment for devices that don't provide labels
		if (vis.name == null || (changed & (SpatialComponent.Marker | SpatialComponent.Label | SpatialComponent.PlaneAlignment)) > 0)
		{
			if (entity.TryGetMarker(out MarkerType markerType, out uint markerId))
				vis.name = entity.MarkerText ?? $"Marker #{markerId}";
			else if (label != SpatialLabel.None)
				vis.name = label.ToString();
			else if (entity.TryGetPlaneAlign(out PlaneAlign align))
				vis.name = align.ToString();
			else
				vis.name = "";
		}
		if (vis.name != "" && hasRect)
			Text.Add(vis.name, center.ToMatrix());

		// Wireframe entity meshes, refilled only when the data changes
		if (showMeshes && (components & SpatialComponent.Mesh) > 0)
		{
			if (vis.mesh == null)
			{
				vis.mesh = new Mesh();
				if (!entity.TryGetMesh(vis.mesh, out vis.meshOrigin))
					vis.mesh = null;
			}
			else
			{
				Mesh refill = (changed & SpatialComponent.Mesh) > 0 ? vis.mesh : null;
				entity.TryGetMesh(refill, out vis.meshOrigin);
			}
			if (vis.mesh != null)
				vis.mesh.Draw(meshMat, vis.meshOrigin.ToMatrix());
		}
	}

	static void TrackSavedId(Visual vis, Guid persistId)
	{
		if (persistId == vis.persistId) return;
		SavedAnchorIds.Remove(vis.persistId);
		SavedAnchorIds.Add   (persistId);
		vis.persistId = persistId;
	}

	static Color32 LabelColor(SpatialLabel label) => label switch {
		SpatialLabel.Floor   => new Color32( 64, 255,  64, 255),
		SpatialLabel.Wall    => new Color32( 64,  64, 255, 255),
		SpatialLabel.Ceiling => new Color32(255,  64,  64, 255),
		SpatialLabel.Table   => new Color32(255, 255,  64, 255),
		_                    => new Color32(255, 255, 255, 255),
	};

	static void DrawRect(Pose pose, Vec2 size, Color32 color)
	{
		Vec3 right = pose.orientation * V.XYZ(size.x * 0.5f, 0, 0);
		Vec3 up    = pose.orientation * V.XYZ(0, size.y * 0.5f, 0);
		Vec3 p     = pose.position;
		Lines.Add(p - right - up, p + right - up, color, 0.005f);
		Lines.Add(p + right - up, p + right + up, color, 0.005f);
		Lines.Add(p + right + up, p - right + up, color, 0.005f);
		Lines.Add(p - right + up, p - right - up, color, 0.005f);
	}

	static void DrawBox(Pose pose, Vec3 size, Color32 color)
	{
		Vec3   h = size * 0.5f;
		Vec3[] c = new Vec3[8];
		for (int i = 0; i < 8; i++)
			c[i] = pose.position + pose.orientation * V.XYZ((i & 1) > 0 ? h.x : -h.x, (i & 2) > 0 ? h.y : -h.y, (i & 4) > 0 ? h.z : -h.z);
		int[,] edges = { {0,1},{1,3},{3,2},{2,0}, {4,5},{5,7},{7,6},{6,4}, {0,4},{1,5},{2,6},{3,7} };
		for (int i = 0; i < 12; i++)
			Lines.Add(c[edges[i, 0]], c[edges[i, 1]], color, 0.005f);
	}
}

// PersistIds only mean something if you keep them! This remembers every
// anchor the app has persisted, so they can be restored or cleaned up in a
// later session.
static class SavedAnchorIds
{
	static HashSet<Guid> ids;
	static string        FilePath => Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "StereoKitTest", "spatial_anchor_ids.txt");

	public static IReadOnlyCollection<Guid> Ids   => Load();
	public static int                       Count => Load().Count;

	public static void Add   (Guid id) { if (id != Guid.Empty && Load().Add   (id)) Save(); }
	public static void Remove(Guid id) { if (id != Guid.Empty && Load().Remove(id)) Save(); }

	// Unpersist works from just the id, so this also cleans up anchors from
	// earlier sessions that haven't been found again.
	public static void Forget()
	{
		foreach (Guid id in Load())
			SpatialEntity.Unpersist(id);
		ids.Clear();
		Save();
	}

	static HashSet<Guid> Load()
	{
		if (ids != null) return ids;
		try   { ids = new HashSet<Guid>(File.ReadAllLines(FilePath).Select(Guid.Parse)); }
		catch { ids = new HashSet<Guid>(); }
		return ids;
	}
	static void Save()
	{
		Directory.CreateDirectory(Path.GetDirectoryName(FilePath));
		File.WriteAllLines(FilePath, ids.Select(id => id.ToString()));
	}
}
