using StereoKit;
using StereoKit.Framework;

class DemoFBPassthrough : ITest
{
	string title       = "FB Passthrough Extension";
	string description = "Passthrough AR!";
	Pose   windowPose  = Demo.contentPose * Pose.Identity;
	PassthroughFBExt passthrough;

	public void Initialize() { passthrough = SK.GetOrCreateStepper<PassthroughFBExt>(); }
	public void Shutdown() { }

	public void Step()
	{
		UI.WindowBegin("Passthrough Settings", ref windowPose);
		bool toggle = passthrough.Enabled;
		UI.Label(passthrough.Available
			? "Passthrough EXT available!"
			: "No passthrough EXT available :(");
		UI.PushEnabled(passthrough.Available);
		if (UI.Toggle("Passthrough", ref toggle))
			passthrough.Enabled = toggle;
		UI.PopEnabled();
		UI.WindowEnd();

		Demo.ShowSummary(title, description);
	}
}