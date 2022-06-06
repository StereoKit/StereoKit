#if WINDOWS_UWP
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using StereoKit;
using Windows.Perception.Spatial;

class DemoWorldAnchor : ITest
{
	SpatialAnchorStore       anchorStore;
	Dictionary<string, Pose> anchorPoses = new Dictionary<string, Pose>();
	Model                    wandModel   = Model.FromFile("Wand.glb", Shader.UI);
	Pose                     wandPose    = new Pose(.5f, 0, -0.3f, Quat.LookDir(1, 0, 0));

	public bool Enabled => true;

	string selected = null;

	public string CreateAnchor(Pose at, string id = null)
	{
		string anchorID = id == null
			? Guid.NewGuid().ToString()
			: id;

		SpatialLocator                    locator    = SpatialLocator.GetDefault();
		SpatialStationaryFrameOfReference stationary = locator.CreateStationaryFrameOfReferenceAtCurrentLocation();
		SpatialAnchor                     anchor     = SpatialAnchor.TryCreateRelativeTo(stationary.CoordinateSystem);
		Pose                              pose       = World.FromPerceptionAnchor(anchor);

		Pose newAnchor = pose.ToMatrix().Inverse.Transform(at);
		anchor = SpatialAnchor.TryCreateRelativeTo(stationary.CoordinateSystem, newAnchor.position, newAnchor.orientation);
		pose   = World.FromPerceptionAnchor(anchor);

		if (anchor != null)
			anchorPoses.Add(anchorID, pose);

		return anchorStore.TrySave(anchorID, anchor)
			? anchorID
			: null;
	}

	public void DestroyAnchor(string id)
	{
		anchorStore.Remove(id);
		anchorPoses.Remove(id);
		if (id == selected)
			selected = null;
	}

	public void Initialize()
	{
		GetStoreAsync();
	}
	private async Task GetStoreAsync()
	{
		anchorStore = await SpatialAnchorManager.RequestStoreAsync();
		LoadAnchors();
	}
	private void LoadAnchors()
	{
		anchorPoses = new Dictionary<string, Pose>();
		IReadOnlyDictionary<string, SpatialAnchor> anchors = anchorStore.GetAllSavedAnchors();
		foreach (var anchor in anchors.ToList())
		{
			if (World.FromPerceptionAnchor(anchor.Value, out Pose at))
			{
				anchorPoses[anchor.Key] = at;
				anchor.Value.RawCoordinateSystemAdjusted += OnCoordinateSystemAdjusted;
			}
		}
	}
	private void OnCoordinateSystemAdjusted(SpatialAnchor sender, SpatialAnchorRawCoordinateSystemAdjustedEventArgs args)
	{
	}

	Pose pose = new Pose(Vec3.Forward * 0.4f, Quat.Identity);
	public void Update()
	{
		Vec3 wandTip = wandModel.Bounds.center + wandModel.Bounds.dimensions.y * 0.5f * Vec3.Up;

		UI.HandleBegin("wand", ref wandPose, wandModel.Bounds);
		wandModel.Draw(Matrix.Identity);
		wandTip = Hierarchy.ToWorld(wandTip);
		UI.HandleEnd();
		Lines.AddAxis(new Pose(wandTip, Quat.Identity), 0.05f);

		UI.WindowBegin("Anchors", ref pose);
		if (anchorStore != null && UI.Button("Create New"))
			CreateAnchor(new Pose(wandTip, Quat.Identity));
		
		// List options for the selected anchor
		if (selected != null)
		{
			UI.HSeparator();
			UI.Label(selected);
			if (UI.Button("Delete")) DestroyAnchor(selected);
		}
		UI.WindowEnd();

		// Show anchors, and do selection logic on them
		foreach (var p in anchorPoses) {
			Lines.AddAxis(p.Value, 0.1f);

			if (p.Value.position.InRadius(wandTip, 0.05f))
				selected = p.Key;
		}
		// Outline the selected anchor
		if (selected != null) {
			Pose p = anchorPoses[selected];
			Mesh.Cube.Draw(Material.UIBox, p.ToMatrix(0.1f));
		}
	}

	public void Shutdown()
	{
	}
}
#endif