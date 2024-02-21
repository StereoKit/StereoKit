using StereoKit;
using StereoKit.Framework;

internal class DemoLaserPointer : ITest
{
	Pose  pose = Demo.contentPose.Pose;
	float sliderVal = 0;

	bool sliderMode = false;

	public void Initialize()
	{
		SK.RemoveStepper<HandMenuRadial>();
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
		UI.WindowBegin("Laser Pointer", ref pose, new Vec2(20, 0) * U.cm);
		UI.Button("Clickable");
		UI.Toggle("Slider Mode", ref sliderMode);
		UI.HSlider("Slider", ref sliderVal, -0.1f, 0.1f, confirmMethod: sliderMode ? UIConfirm.Push : UIConfirm.Pinch);
		UI.VSlider("VSlider", ref sliderVal, -0.1f, 0.1f, 0, 0.1f, confirmMethod: sliderMode ? UIConfirm.Push : UIConfirm.Pinch);

		UI.WindowEnd();
	}
}

class ControllerStepper : IStepper
{
	Model controllerL;
	Model controllerR;

	public bool Enabled => true;

	public bool Initialize()
	{
		controllerL = Model.FromFile("ControllerLeft.glb");
		controllerR = Model.FromFile("ControllerRight.glb");
		return true;
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
		if (Input.HandSource(Handed.Left) != HandSource.Articulated)
		{
			Controller l = Input.Controller(Handed.Left);
			if (l.tracked.IsActive())
			{
				controllerL.Draw(l.aim.ToMatrix());
			}

			Controller r = Input.Controller(Handed.Right);
			if (r.tracked.IsActive())
			{
				controllerR.Draw(r.aim.ToMatrix());
			}
		}
	}
}