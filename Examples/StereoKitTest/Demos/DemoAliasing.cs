using StereoKit;

class DemoAliasing : ITest
{
	string title       = "Aliasing";
	string description = "...";

	Pose windowPose = Demo.contentPose.Pose;

	float multisample;
	float scaling;

	public void Initialize()
	{
		multisample = Renderer.Multisample;
		scaling     = Renderer.Scaling;
	}
	public void Shutdown() => Microphone.Stop();
	public void Update()
	{
		UI.WindowBegin("Aliasing Settings", ref windowPose);

		UI.Label("Swapchaing scaling");
		UI.Label($"{scaling:0.00}", V.XY(0.04f,0));
		UI.SameLine();
		UI.HSlider("scaling", ref scaling, 0.1f, 2, 0.05f);

		UI.HSeparator();

		UI.Label("MSAA");
		UI.Label($"{(int)multisample}", V.XY(0.04f, 0));
		UI.SameLine();
		UI.HSlider("scaling", ref multisample, 1, 8, 1);

		if(UI.Button("Confirm"))
		{
			Renderer.Multisample = (int)multisample;
			Renderer.Scaling     = scaling;
		}

		UI.WindowEnd();

		Demo.ShowSummary(title, description);
	}

}