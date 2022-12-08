using StereoKit;
using System;

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

		if (Backend.XRType != BackendXRType.OpenXR)
		{
			UI.Label("These settings are only available in XR");
			UI.PushEnabled(false);
		}

		UI.Label("Swapchain scaling");
		UI.Label($"{scaling:0.00}", V.XY(0.04f,0));
		UI.SameLine();
		UI.HSlider("scaling", ref scaling, 0.1f, 2, 0.05f);

		UI.HSeparator();

		UI.Label("MSAA");
		UI.Label($"{(int)multisample}", V.XY(0.04f, 0));
		UI.SameLine();
		UI.HSlider("msaa", ref multisample, 1, 8, 1);

		if (Input.Key(Key.Right).IsJustActive()) scaling = Math.Min(scaling + 0.1f, 2);
		if (Input.Key(Key.Left ).IsJustActive()) scaling = Math.Max(scaling - 0.1f, 0.1f);
		if (Input.Key(Key.Up   ).IsJustActive()) multisample = Math.Min(multisample + 1, 8);
		if (Input.Key(Key.Down ).IsJustActive()) multisample = Math.Max(multisample - 1, 1);

		if (UI.Button("Confirm") || Input.Key(Key.Space).IsJustActive())
		{
			Renderer.Multisample = (int)multisample;
			Renderer.Scaling     = scaling;
		}

		if (Backend.XRType != BackendXRType.OpenXR)
			UI.PopEnabled();

		UI.WindowEnd();

		Demo.ShowSummary(title, description);
	}

}