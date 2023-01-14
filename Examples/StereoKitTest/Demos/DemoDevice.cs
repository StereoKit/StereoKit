using StereoKit;

class DemoDevice : ITest
{
	string title       = "Device";
	string description = "The Device class contains a number of interesting bits of data about the device it's running on! Most of this is just information, but there's a few properties that can also be modified.";

	Pose windowPose = Demo.contentPose.Pose;

	public void Initialize() { }
	public void Shutdown  () { }

	public void Update() {
		UI.WindowBegin("Device Info", ref windowPose);

		Vec2 labelSize = V.XY(0.06f,0);
		UI.Label("Name", labelSize); UI.SameLine();
		UI.Label(Device.Name);
		UI.Label("GPU", labelSize); UI.SameLine();
		UI.Label(Device.GPU);
		UI.Label("Tracking", labelSize); UI.SameLine();
		UI.Label(Device.Tracking.ToString());
		UI.Label("Display", labelSize); UI.SameLine();
		UI.Label(Device.DisplayType.ToString());
		UI.Label("Blend", labelSize); UI.SameLine();
		UI.Label(Device.DisplayBlend.ToString());

		UI.HSeparator();

		labelSize = V.XY(0.12f, 0);
		UI.Label("Has Eye Gaze", labelSize); UI.SameLine();
		UI.Label(Device.HasEyeGaze.ToString());
		UI.Label("Has Hand Tracking", labelSize); UI.SameLine();
		UI.Label(Device.HasHandTracking.ToString());

		UI.WindowEnd();
		Demo.ShowSummary(title, description);
	}
}