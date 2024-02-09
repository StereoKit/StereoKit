using StereoKit;
using StereoKit.Framework;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

internal class DemoLaserPointer : ITest
{
	Model controllerL;
	Model controllerR;
	Pose  pose = Demo.contentPose.Pose;
	float sliderVal = 0;

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
		Pointer pl = Input.Pointer(0, InputSource.HandLeft);
		if (pl.tracked.IsActive()) {
			controllerL.Draw(pl.Pose.ToMatrix());
			Lines.Add(pl.ray.position, pl.ray.At(0.5f), new Color32(255, 255, 255, 255), new Color32(255, 255, 255, 0), 0.005f);
		}

		Pointer pr = Input.Pointer(0, InputSource.HandRight);
		if (pr.tracked.IsActive()) {
			controllerR.Draw(pr.Pose.ToMatrix());
			Lines.Add(pr.ray.position, pr.ray.At(0.5f), new Color32(255,255,255,255), new Color32(255, 255, 255, 0), 0.005f);
		}

		UI.WindowBegin("Laser Pointer", ref pose, new Vec2(20, 20) * U.cm);
		UI.Button("Clickable");
		UI.HSlider("Slider", ref sliderVal, -0.1f, 0.1f);
		UI.WindowEnd();
	}
}
