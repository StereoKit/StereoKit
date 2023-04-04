
using StereoKit;

class DemoUIGrabBar : ITest
{
	string title       = "UI Grab Bar";
	string description = "This is an example of using external handles to manipulate a Window's pose! Since _you_ own the Pose data, you can do whatever you want with it!\nThe grab bar below the window is a common sight to see in recent XR UI, so here's one way to replicate that with SK's API.";

	public void Initialize()
	{
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
		Tests.Screenshot("UIGrabBarWindow.jpg", 1, 400, 400, 90, grabBarPose.position + V.XYZ(-0.2f, -0.23f, 0.2f), grabBarPose.position - V.XYZ(0, 0.23f, 0));
		GrabBarWindow();
		Demo.ShowSummary(title, description);
	}

	Pose grabBarPose = new Pose(0.5f, 0, -0.4f, Quat.LookDir(-1, 0, 1));
	Vec2 windowSize  = Vec2.Zero; // First frame will not be in the right spot unless we know window size in advance.
	public void GrabBarWindow()
	{
		// Use the size of the Window to position it directly above the grab
		// bar. Note that we're using the Pose relative Up direction instead of
		// just the Y axis.
		Pose windowPose = grabBarPose;
		windowPose.position -= grabBarPose.Up * windowSize.y;
		// Begin the window, only show the body, and don't allow the user to
		// grab it. That'd require a bit more work :)
		UI.WindowBegin("Interface", ref windowPose, new Vec2(0.15f, 0), UIWin.Body, UIMove.None);

		// Some UI to fill up space
		UI.Text("Here's a window that can be moved via a bar below the window!", TextAlign.Center);
		UI.HSeparator();
		UI.Button("Testing!");

		// Save the size of the window before ending it. MUST be on a new line.
		windowSize = new Vec2(UI.LayoutRemaining.x, UI.LayoutAt.y - UI.Settings.margin);
		UI.WindowEnd();

		// Show the window grab bar at its own location.
		UI.Handle("GrabBar", ref grabBarPose, new Bounds(new Vec3(windowSize.x * 0.9f, 0.01f, 0.01f)), true, UIMove.FaceUser);
	}
}