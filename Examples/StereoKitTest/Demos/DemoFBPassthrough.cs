using StereoKit;

class DemoFBPassthrough : ITest
{
	string title       = "FB Passthrough Extension";
	string description = "Passthrough AR!";
	Pose   windowPose  = Demo.contentPose * Pose.Identity;

	public void Initialize() { }
	public void Shutdown() { }

	public void Step()
	{
		UI.WindowBegin("Passthrough Settings", ref windowPose);
		bool toggle = App.passthrough.EnabledPassthrough;
		UI.Label(App.passthrough.Available
			? "Passthrough EXT available!"
			: "No passthrough EXT available :(");
		if (UI.Toggle("Passthrough", ref toggle))
			App.passthrough.EnabledPassthrough = toggle;
		UI.WindowEnd();

		Demo.ShowSummary(title, description);
	}
}