using StereoKit;
using StereoKit.Framework;

internal class DemoLaserPointer : ITest
{
	Model controllerL;
	Model controllerR;
	Pose  pose = Demo.contentPose.Pose;
	float sliderVal = 0;

	bool sliderMode = false;

	public void Initialize()
	{
		controllerL = Model.FromFile("ControllerLeft.glb");
		controllerR = Model.FromFile("ControllerRight.glb");
		Input.HandVisible(Handed.Right, false);
		Input.HandVisible(Handed.Left, false);
		SK.RemoveStepper<HandMenuRadial>();
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
		//if (Input.HandSource(Handed.Left) == HandSource.None) {
		{
			Pointer pl = Input.Pointer(0, InputSource.HandLeft);
			if (pl.tracked.IsActive()) {
				controllerL.Draw(pl.Pose.ToMatrix());
			}

			Pointer pr = Input.Pointer(0, InputSource.HandRight);
			if (pr.tracked.IsActive()) {
				controllerR.Draw(pr.Pose.ToMatrix());
			}
		}

		UI.WindowBegin("Laser Pointer", ref pose, new Vec2(20, 0) * U.cm);
		UI.Button("Clickable");
		UI.Toggle("Slider Mode", ref sliderMode);
		UI.HSlider("Slider", ref sliderVal, -0.1f, 0.1f, confirmMethod: sliderMode ? UIConfirm.Push : UIConfirm.Pinch);
		UI.VSlider("VSlider", ref sliderVal, -0.1f, 0.1f, 0, 0.1f, confirmMethod: sliderMode ? UIConfirm.Push : UIConfirm.Pinch);

		UI.WindowEnd();
	}
}
