using System;
using System.Collections.Generic;
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
	}
	Dictionary<uint, Visual> visuals = new Dictionary<uint, Visual>();

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

		SpatialEntity.Enable(SpatialEntity.Capabilities);
	}

	public void Shutdown()
	{
		SpatialEntity.Disable(SpatialEntity.Capabilities);
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

		UI.HSeparator();
		UI.Label($"{SpatialEntity.Count()} entities");
		UI.Toggle("Show Meshes", ref showMeshes);

		// Anchor creation + persistence, when the capability is up
		UI.HSeparator();
		UI.PushEnabled((SpatialEntity.Active & SpatialCapability.Anchor) > 0);
		if (UI.Button("Create Anchor"))
		{
			Pose head = Input.Head;
			SpatialEntity anchor = SpatialEntity.CreateAnchor(new Pose(head.position + head.Forward * 0.5f, Quat.LookAt(head.position + head.Forward * 0.5f, head.position)));
			if (anchor.Valid && persistNew)
				anchor.Persist();
		}
		UI.SameLine();
		UI.PushEnabled(SpatialEntity.PersistenceAvailable);
		UI.Toggle("Persist", ref persistNew);
		UI.PopEnabled();
		UI.PopEnabled();
		UI.WindowEnd();

		foreach (SpatialEntity entity in SpatialEntity.All)
			DrawEntity(entity);

		Demo.ShowSummary(title, description,
			new Bounds(V.XY0(0.2f, -0.06f), V.XYZ(.3f, .28f, 0.1f)));
	}

	void DrawEntity(SpatialEntity entity)
	{
		if (!visuals.TryGetValue(entity.Id, out Visual vis))
		{
			vis = new Visual();
			visuals.Add(entity.Id, vis);
		}
		SpatialComponent components = entity.Components;
		SpatialComponent changed    = entity.Changed;
		bool             tracked    = entity.Tracked.IsActive();

		entity.TryGetPlane(out PlaneAlign align, out PlaneLabel label);
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
			}
			Text.Add(vis.anchorLabel, anchorPose.ToMatrix(), Pivot.TopCenter);
		}

		// A text label: decoded marker data, marker id, or plane label
		if (vis.name == null || (changed & (SpatialComponent.Marker | SpatialComponent.PlaneLabel)) > 0)
		{
			if (entity.TryGetMarker(out MarkerType markerType, out uint markerId))
				vis.name = entity.MarkerText ?? $"Marker #{markerId}";
			else if (label != PlaneLabel.None)
				vis.name = label.ToString();
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

	static Color32 LabelColor(PlaneLabel label) => label switch {
		PlaneLabel.Floor   => new Color32( 64, 255,  64, 255),
		PlaneLabel.Wall    => new Color32( 64,  64, 255, 255),
		PlaneLabel.Ceiling => new Color32(255,  64,  64, 255),
		PlaneLabel.Table   => new Color32(255, 255,  64, 255),
		_                  => new Color32(255, 255, 255, 255),
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
