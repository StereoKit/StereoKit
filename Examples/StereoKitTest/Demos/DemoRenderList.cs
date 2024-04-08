using StereoKit;

class DemoRenderList : ITest
{
	string title       = "Render Lists";
	string description = "";

	public void Step()
	{
		ShowTextInputWindow();

		Demo.ShowSummary(title, description, new Bounds(V.XY0(0, -0.19f), V.XYZ(.4f, .5f, 0)));
	}

	Pose winPose = Pose.Identity;
	bool clear   = false;
	void ShowTextInputWindow()
	{
		UI.WindowBegin("Render Lists", ref winPose);
		UI.Label($"Render items: {RenderList.Primary.PrevCount}");
		UI.Toggle("Clear", ref clear);
		UI.WindowEnd();

		if (clear) RenderList.Primary.Clear();
	}

	public void Initialize()
	{
		winPose = (Demo.contentPose * winPose.ToMatrix()).Pose;
	}
	public void Shutdown() { }
}