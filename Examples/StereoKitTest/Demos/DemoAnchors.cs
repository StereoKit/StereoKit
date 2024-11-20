using System.Collections.Generic;
using StereoKit;

class DemoAnchors : ITest
{
	string title       = "Anchors";
	string description = "This demo uses StereoKit's Anchor API to add, remove, and load spatial anchors that are locked to local physical locations. These can be used for persisting locations across sessions, or increasing the stability of your experiences!";

	List<Anchor> anchors   = new List<Anchor>();
	Anchor       selected  = null;
	Model        wandModel = Model.FromFile("Wand.glb", Shader.UI);
	Pose         wandPose  = new Pose(Demo.contentPose.Translation + V.XY0(0.2f,0), Quat.Identity);

	public bool Enabled => true;

	public void Initialize() => anchors.AddRange(Anchor.Anchors);
	public void Shutdown  () => anchors.Clear();

	Pose pose = Demo.contentPose.Pose;
	public void Step()
	{
		// Draw a wand for placing and selecting anchors
		Vec3 wandTip = wandModel.Bounds.center + wandModel.Bounds.dimensions.y * 0.5f * Vec3.Up;
		UI.HandleBegin("wand", ref wandPose, wandModel.Bounds);
		wandModel.Draw(Matrix.Identity);
		wandTip = Hierarchy.ToWorld(wandTip);
		UI.HandleEnd();
		Lines.AddAxis(new Pose(wandTip, Quat.Identity), 0.05f);

		// Window for working with the anchors
		UI.WindowBegin("Anchors", ref pose);

		// Not all systems support anchors, or all features of anchors, so
		// we're checking for that and displaying the info here!
		UI.LayoutPushCut(UICut.Left, 0.1f);
		UI.PanelAt(UI.LayoutAt, UI.LayoutRemaining);
		UI.Label("Capabilities:");
		UI.HSeparator();
		bool storable  = (Anchor.Capabilities & AnchorCaps.Storable)  > 0;
		bool stability = (Anchor.Capabilities & AnchorCaps.Stability) > 0;
		if (storable ) UI.Label("Storable");
		if (stability) UI.Label("Stability");
		if (!storable && !stability) UI.Label("None");
		UI.LayoutPop();

		// Add a new anchor at the wand tip
		UI.PushEnabled(storable || stability);
		if (UI.Button("Create New"))
		{
			Anchor anchor = Anchor.FromPose(new Pose(wandTip, Quat.Identity));
			anchor.TrySetPersistent(true);
			anchors.Add(anchor);
		}
		UI.PopEnabled();

		// List options for the selected anchor
		UI.PushEnabled(selected != null);
		UI.HSeparator();
		UI.Label(selected?.Name ?? "None selected");
		if (UI.Button("Delete"))
		{
			selected.TrySetPersistent(false);
			anchors.Remove(selected);
			selected = null;
		}
		UI.PopEnabled();

		UI.WindowEnd();

		// Show where all the anchors are located, and select them if the wand
		// tip is within a certain radius.
		foreach (var p in anchors)
		{
			Lines.AddAxis(p.Pose, 0.1f);

			if (p.Pose.position.InRadius(wandTip, 0.05f))
				selected = p;
		}
		// Outline the selected anchor.
		if (selected != null)
			Mesh.Cube.Draw(Material.UIBox, selected.Pose.ToMatrix(0.1f));

		// Log to the console whenever a new anchor is discovered.
		foreach (Anchor a in Anchor.NewAnchors)
			Log.Info($"New anchor: {a.Name}");

		Demo.ShowSummary(title, description,
			new Bounds(V.XY0(-0.075f, 0.02f), V.XYZ(.45f, .38f, 0.1f)));
	}


}