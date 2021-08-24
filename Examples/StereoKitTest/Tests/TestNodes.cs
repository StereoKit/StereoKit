using StereoKit;

class TestNodes : ITest
{
	Pose      modelPose = Pose.Identity;
	Model     testModel;
	ModelNode surfaceNode;

	public void Initialize() {
		testModel = new Model();
		testModel
			.AddNode ("Root",    Matrix.S(0.2f), Mesh.Cube, Material.Default)
			.AddChild("Sub",     Matrix.TR(V.XYZ(0.5f, 0, 0), Quat.FromAngles(0, 0, 45)), Mesh.Cube, Material.Default)
			.AddChild("Surface", Matrix.TRS(V.XYZ(0.5f, 0, 0), Quat.LookDir(V.XYZ(1,0,0)), V.XYZ(1,1,1)));

		surfaceNode = testModel.FindNode("Surface");
	}
	public void Shutdown() { }
	public void Update()
	{
		UI.PushSurface(modelPose);
		testModel.Draw(Matrix.Identity, new Color(0.5f, 0.5f, 0.5f));
		UI.PushSurface(
			surfaceNode.ModelTransform.Pose,
			surfaceNode.ModelTransform.Scale.XY0/2,
			surfaceNode.ModelTransform.Scale.XY);
		Vec3 center = Hierarchy.ToWorld(Vec3.Zero);
		UI.Label("UI on a Model node!");
		UI.HSeparator();
		UI.Button("Do Thing");
		UI.PopSurface();
		UI.PopSurface();

		Tests.Screenshot(400, 400, "Tests/NodeUI.jpg", center+Vec3.One*0.15f, center);
	}
}