using StereoKit;

// Grabbing a UIMove.FaceUser window that's turned away snaps it to face the
// user on the grab frame. That snap must not press a button in the same pinch.
// A far ray from a hand low and to the side pinches the empty margin right
// beside a button on a window pitched 45 degrees away. After the snap the
// button's raised plate crosses the oblique ray, so the button must not go
// active on the grab frame or fire on release.
//
// Frame 0 is a warm-up for a freshly created interactor (see TestUIFocus), then
// frames 1-3 focus the window body, pinch, and release.
class TestUIFaceUserGrab : ITest
{
	static readonly Vec3 handAt   = V.XYZ(-0.3f, -0.1f, -0.1f);
	static readonly Vec3 grabAt   = V.XYZ(0.115f, -0.035f, 0); // window space, left margin beside B00
	static readonly Pose restPose = new Pose(0, 0, -0.5f, Quat.FromAngles(45, 180, 0));

	Pose windowPose = restPose;
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
			BtnState pinch = frame == 2 ? BtnState.Active | BtnState.JustActive : BtnState.Inactive;
			Vec3     target = windowPose.ToMatrix().Transform(grabAt);
			Vec3     dir    = (target - handAt).Normalized;
			ray.Update(handAt, handAt + dir * 100, new Pose(handAt, Quat.LookDir(dir)), handAt, Vec3.Zero, pinch, BtnState.Active);
		}

		UI.WindowBegin("FaceUser", ref windowPose, V.XY(0.24f, 0.2f), UIWin.Normal, UIMove.FaceUser);
		BtnState winActive = UI.LastElementActive;
		bool pressed = false, anyButtonActive = false;
		for (int y = 0; y < 3; y++)
		{
			for (int x = 0; x < 2; x++)
			{
				if (x > 0) UI.SameLine();
				if (UI.Button($"B{x}{y}", V.XY(0.08f, 0.04f))) pressed = true;
				if (UI.LastElementActive.IsActive()) anyButtonActive = true;
			}
		}
		UI.WindowEnd();

		if (scripted)
		{
			Log.Info($"frame {frame}: window active=[{winActive}] button active={anyButtonActive} pressed={pressed}");
			if (frame == 2) { grabbed = winActive.IsJustActive(); buttonActivated = anyButtonActive; }
			if (frame == 3) { buttonFired = pressed; }
			if (frame == 3)
			{
				Tests.Test(GrabHappens);
				Tests.Test(GrabDoesNotActivateButton);
				Tests.Test(GrabDoesNotFireButton);
			}
		}
		frame++;
	}

	// Control: the pinch really does grab the window body.
	bool GrabHappens()               => grabbed;
	bool GrabDoesNotActivateButton() => !buttonActivated;
	bool GrabDoesNotFireButton()     => !buttonFired;

	public void Shutdown()
	{
		if (Tests.IsTesting) ray.Destroy();
		Interaction.DefaultInteractors = oldInteractors;
	}
}
