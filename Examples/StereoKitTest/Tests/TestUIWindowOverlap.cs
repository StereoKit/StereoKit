using StereoKit;

// A click on the empty body of a window must grab that window, never a button
// on a window behind it. The back window is deliberately drawn *before* the
// front one: its button claims focus first in the frame, and a click must not
// stick to that provisional claim once the closer window body shows up.
//
// One far ray fires straight along the view axis, through the front window's
// body, at a button on the back window. Frame 0 is a warm-up for a freshly
// created interactor (see TestUIFocus), then frames 1-3 focus, pinch, release.
class TestUIWindowOverlap : ITest
{
	Pose frontPose = new Pose(0, 0, -0.5f, Quat.LookDir(0, 0, 1));
	Pose backPose  = new Pose(0, 0, -0.6f, Quat.LookDir(0, 0, 1));

	static readonly Vec3 buttonAt   = V.XYZ(0.1f, -0.01f, 0); // back window space, top-left
	static readonly Vec2 buttonSize = V.XY(0.2f, 0.05f);

	DefaultInteractors oldInteractors;
	Interactor ray;
	int        frame = 0;

	bool grabbed, buttonActivated, buttonFired;

	public void Initialize()
	{
		oldInteractors = Interaction.DefaultInteractors;
		if (!Tests.IsTesting) return; // leave the default interactors on to try by hand

		Interaction.DefaultInteractors = DefaultInteractors.None;
		ray = Interactor.Create(InteractorType.Line, InteractorEvent.Pinch | InteractorEvent.Poke, InteractorActivation.State, InteractorSource.Unique, 0.01f, 0);
		Tests.RunForFrames(4);
	}

	public void Step()
	{
		bool scripted = Tests.IsTesting && frame < 4;
		if (scripted)
		{
			BtnState pinch  = frame == 2 ? BtnState.Active | BtnState.JustActive : BtnState.Inactive;
			Vec3     target = backPose.ToMatrix().Transform(buttonAt - V.XYZ(buttonSize.x / 2, buttonSize.y / 2, 0));
			Vec3     origin = target + V.XYZ(0, 0, 0.35f);
			Vec3     dir    = V.XYZ(0, 0, -1);
			ray.Update(origin, origin + dir * 100, new Pose(origin, Quat.LookDir(dir)), origin, Vec3.Zero, pinch, BtnState.Active);
		}

		UI.WindowBegin("Back", ref backPose, V.XY(0.24f, 0.2f), UIWin.Body);
		bool     pressed      = UI.ButtonAt("Behind", buttonAt, buttonSize);
		BtnState buttonActive = UI.LastElementActive;
		UI.WindowEnd();

		UI.WindowBegin("Front", ref frontPose, V.XY(0.2f, 0.1f), UIWin.Body);
		BtnState frontActive = UI.LastElementActive;
		UI.WindowEnd();

		if (scripted)
		{
			Log.Info($"frame {frame}: front window active=[{frontActive}] back button active=[{buttonActive}] pressed={pressed}");
			if (frame == 2) { grabbed = frontActive.IsJustActive(); buttonActivated = buttonActive.IsActive(); }
			if (frame == 3) { buttonActivated |= buttonActive.IsActive(); buttonFired = pressed; }
			if (frame == 3)
			{
				Tests.Test(FrontWindowGrabbed);
				Tests.Test(BackButtonNotActivated);
				Tests.Test(BackButtonNotFired);
			}
		}
		frame++;
	}

	// Control: the pinch lands on the front window body.
	bool FrontWindowGrabbed()     => grabbed;
	bool BackButtonNotActivated() => !buttonActivated;
	bool BackButtonNotFired()     => !buttonFired;

	public void Shutdown()
	{
		if (Tests.IsTesting) ray.Destroy();
		Interaction.DefaultInteractors = oldInteractors;
	}
}
