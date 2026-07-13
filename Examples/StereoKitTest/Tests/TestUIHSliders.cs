using StereoKit;
using System;

class TestUIHSliders : ITest
{
	float pushValue  = 0;
	float pinchValue = 0;

	DefaultInteractors oldInteractors;

	DemoAnim<HandJoint[]> anim;

	bool TestPush()
	{
		bool match = Math.Abs(pushValue - 0.14f) < 0.02f;
		Log.Write(match ? LogLevel.Info : LogLevel.Error, $"HSlider push value: {pushValue:N3} - expected ~0.14");
		return match;
	}

	bool TestPinch()
	{
		bool match = Math.Abs(pinchValue - 0.5f) < 0.02f;
		Log.Write(match ? LogLevel.Info : LogLevel.Error, $"HSlider pinch value: {pinchValue:N3} - expected ~0.5");
		return match;
	}

	public void Initialize() {
		oldInteractors = Interaction.DefaultInteractors;
		Interaction.DefaultInteractors = DefaultInteractors.All;

		anim = new DemoAnim<HandJoint[]>(DebugToolWindow.JointsLerp, DebugToolWindow.ReadHandAnim("test_hsliders_anim.bin"));

		if (Tests.IsTesting) {
			const float playbackSpeed = 4;

			Input.HandVisible(Handed.Max, true);
			anim.Play(playbackSpeed);
			Tests.RunForSeconds(anim.Duration / playbackSpeed);
		}
	}

	public void Shutdown() {
		if (Tests.IsTesting)
			Input.HandClearOverride(Handed.Right);

		Tests.Test(TestPush);
		Tests.Test(TestPinch);

		Interaction.DefaultInteractors = oldInteractors;
	}
	public void Step()
	{
		if (anim.Playing)
			Input.HandOverride(Handed.Right, anim.Current);

		Pose pose = new Pose(V.X0Z(0,-0.5f), Quat.LookDir(-Vec3.Forward));
		UI.WindowBegin("Slider Test", ref pose);
		UI.HSlider("Push",  ref pushValue,  0, 1, 0, 0.15f, UIConfirm.Push);
		UI.HSlider("Pinch", ref pinchValue, 0, 1, 0, 0.15f, UIConfirm.Pinch);
		UI.WindowEnd();
	}
}