using StereoKit;

class DemoWelcome : ITest
{
	Model  logo;
	string message = "Welcome fellow developer!\n\nThis is the StereoKit test app, a collection of demos and tests that cover StereoKit's major features. Use this panel to navigate around the app, and enjoy!\n\nCheck behind you for some debugging tools :)";

	public void Initialize()
	{
		logo = Model.FromFile("StereoKit.glb");
	}

	public void Update()
	{
		Hierarchy.Push(Matrix.T(-0.8f, 0.05f, -0.7f));
		float scale = 1.3f;
		Vec3  size  = logo.Bounds.dimensions * scale;
		logo.Draw(Matrix.TRS(V.XYZ(size.x/2.0f, size.y/2.0f + 0.05f, 0), Quat.LookDir(0, 0, 1), scale));
		Text.Add(message, Matrix.TRS(Vec3.Zero, Quat.LookDir(0, 0, 1), 1.25f), V.XY(.4f, 0), TextFit.Wrap, TextAlign.TopLeft, TextAlign.TopLeft);
		Hierarchy.Pop();
	}

	public void Shutdown()
	{
	}
}