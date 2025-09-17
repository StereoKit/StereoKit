using StereoKit;

class TestUIWindowAutoPose : ITest
{
	public void Initialize()
	{
		Tests.RunForFrames(2);
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
		UI.WindowBegin("Auto Posed Window");
		UI.Text("This window's pose is automatically picked, and maintained internally for extra easy windows.");
		UI.WindowEnd();

		Tests.Screenshot("UI/WindowAutoPose.jpg", 1, 400, 400, 30, new Vec3(0, -0.1f, 0), new Vec3(0, -0.1f, -0.5f));
	}
}