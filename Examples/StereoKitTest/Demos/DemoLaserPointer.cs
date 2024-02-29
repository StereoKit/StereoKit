using StereoKit;
using StereoKit.Framework;

internal class DemoLaserPointer : ITest
{
	Pose  pose = Demo.contentPose.Pose;
	float sliderVal = 0;

	bool sliderMode = false;

	public void Initialize()
	{
		//SK.RemoveStepper<HandMenuRadial>();
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
	enum Mode
	{
		None,
		Mouse,
		Hand,
		Controller,
	}

	Mode  mode = Mode.Hand;
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

	void SetMode(Mode nextMode)
	{
		if (mode == nextMode) return;

		switch(mode)
		{
			case Mode.Mouse: break;
			case Mode.Hand: Input.HandVisible(Handed.Max, false); break;
			case Mode.Controller: break;
		}
		mode = nextMode;

		switch (mode)
		{
			case Mode.Mouse: break;
			case Mode.Hand: Input.HandVisible(Handed.Max, true); break;
			case Mode.Controller: break;
		}
	}

	public void Step()
	{
		if      (Device.DisplayType == DisplayType.Flatscreen)            SetMode(Mode.Mouse);
		else if (Input.HandSource(Handed.Left) == HandSource.Articulated) SetMode(Mode.Hand);
		else                                                              SetMode(Mode.Controller);

		if (mode == Mode.Controller)
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