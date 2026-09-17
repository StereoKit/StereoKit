using StereoKit;

// UI elements that can't be interacted with must still occlude what's behind
// them. Three far-ray interactors fire straight through a stack of windows at
// buttons on a back window:
//   ray 1 -> a disabled button on a movable front window, over back button B1
//   ray 2 -> the empty body of a UIMove.None front window, over back button B2
//   ray 3 -> back button B3, with nothing in front (control)
//   ray 4 -> a UI.BlockAt volume placed in front of back button B4
// Only B3 may focus or activate, and the front window must not be grabbed
// through its disabled button.
//
// Frame 0 is a warm-up for freshly created interactors (see TestUIFocus), then
// frames 1-3 focus, pinch, and hold.
class TestUIBlocking : ITest
{
	Pose frontPose  = new Pose(0,  0,    -0.5f, Quat.LookDir(0, 0, 1));
	Pose coverPose  = new Pose(0, -0.1f, -0.5f, Quat.LookDir(0, 0, 1));
	Pose backPose   = new Pose(0,  0,    -0.6f, Quat.LookDir(0, 0, 1));

	// Back window buttons, top-left corners in window space
	static readonly Vec3 b1At = V.XYZ(0.1f, -0.01f, 0);
	static readonly Vec3 b2At = V.XYZ(0.1f, -0.12f, 0);
	static readonly Vec3 b3At = V.XYZ(0.1f, -0.25f, 0);
	static readonly Vec3 b4At = V.XYZ(0.1f, -0.32f, 0);
	static readonly Vec2 bSize = V.XY(0.2f, 0.05f);

	DefaultInteractors oldInteractors;
	Interactor         ray1, ray2, ray3, ray4;
	int                frame = 0;

	const int frames = 4;
	BtnState[] frontWinActive = new BtnState[frames];
	BtnState[] disabledActive = new BtnState[frames];
	BtnState[] b1Focused = new BtnState[frames], b1Active = new BtnState[frames];
	BtnState[] b2Focused = new BtnState[frames], b2Active = new BtnState[frames];
	BtnState[] b3Focused = new BtnState[frames], b3Active = new BtnState[frames];
	BtnState[] b4Focused = new BtnState[frames], b4Active = new BtnState[frames];

	public void Initialize()
	{
		oldInteractors = Interaction.DefaultInteractors;
		if (!Tests.IsTesting) return; // leave the default interactors on to try by hand

		Interaction.DefaultInteractors = DefaultInteractors.None;
		ray1 = Interactor.Create(InteractorType.Line, InteractorEvent.Pinch | InteractorEvent.Poke, InteractorActivation.State, InteractorSource.Unique, 0.01f, 0);
		ray2 = Interactor.Create(InteractorType.Line, InteractorEvent.Pinch | InteractorEvent.Poke, InteractorActivation.State, InteractorSource.Unique, 0.01f, 0);
		ray3 = Interactor.Create(InteractorType.Line, InteractorEvent.Pinch | InteractorEvent.Poke, InteractorActivation.State, InteractorSource.Unique, 0.01f, 0);
		ray4 = Interactor.Create(InteractorType.Line, InteractorEvent.Pinch | InteractorEvent.Poke, InteractorActivation.State, InteractorSource.Unique, 0.01f, 0);

		Tests.RunForFrames(frames);
	}

	public void Step()
	{
		bool scripted = Tests.IsTesting && frame < frames;
		if (scripted)
		{
			BtnState pinch = frame switch {
				0 => BtnState.Inactive,
				1 => BtnState.Inactive,
				2 => BtnState.Active | BtnState.JustActive,
				_ => BtnState.Active };
			Aim(ray1, ButtonCenter(b1At), pinch);
			Aim(ray2, ButtonCenter(b2At), pinch);
			Aim(ray3, ButtonCenter(b3At), pinch);
			Aim(ray4, ButtonCenter(b4At), pinch);
		}

		// A user blocker floating in world space in front of B4
		UI.BlockAt(new Bounds(ButtonCenter(b4At) + V.XYZ(0, 0, 0.1f), V.XYZ(bSize.x, bSize.y, 0.02f)));

		// Movable front window whose only content is a disabled button
		UI.WindowBegin("Front", ref frontPose, V.XY(0.2f, 0.08f), UIWin.Body, UIMove.FaceUser);
		BtnState fwA = UI.LastElementActive;
		UI.PushEnabled(false);
		UI.Button("Disabled", V.XY(0.18f, 0.05f));
		BtnState dA = UI.LastElementActive;
		UI.PopEnabled();
		UI.WindowEnd();

		// Non-movable front window with an empty body
		UI.WindowBegin("Cover", ref coverPose, V.XY(0.2f, 0.1f), UIWin.Body, UIMove.None);
		UI.WindowEnd();

		// Back window, buttons B1 and B2 sit behind the front windows, B3 is exposed
		UI.WindowBegin("Back", ref backPose, V.XY(0.24f, 0.4f), UIWin.Body, UIMove.None);
		UI.ButtonAt("B1", b1At, bSize); BtnState b1F = UI.LastElementFocused, b1A = UI.LastElementActive;
		UI.ButtonAt("B2", b2At, bSize); BtnState b2F = UI.LastElementFocused, b2A = UI.LastElementActive;
		UI.ButtonAt("B3", b3At, bSize); BtnState b3F = UI.LastElementFocused, b3A = UI.LastElementActive;
		UI.ButtonAt("B4", b4At, bSize); BtnState b4F = UI.LastElementFocused, b4A = UI.LastElementActive;
		UI.WindowEnd();

		if (scripted)
		{
			frontWinActive[frame] = fwA; disabledActive[frame] = dA;
			b1Focused[frame] = b1F; b1Active[frame] = b1A;
			b2Focused[frame] = b2F; b2Active[frame] = b2A;
			b3Focused[frame] = b3F; b3Active[frame] = b3A;
			b4Focused[frame] = b4F; b4Active[frame] = b4A;
			Log.Info($"frame {frame}: front window active=[{fwA}] disabled button active=[{dA}]");
			Log.Info($"  B1 behind disabled button: focused=[{b1F}] active=[{b1A}]");
			Log.Info($"  B2 behind UIMove.None body: focused=[{b2F}] active=[{b2A}]");
			Log.Info($"  B3 exposed:                 focused=[{b3F}] active=[{b3A}]");
			Log.Info($"  B4 behind UI.BlockAt:       focused=[{b4F}] active=[{b4A}]");

			if (frame == frames - 1)
			{
				Tests.Test(ExposedButtonActivates);
				Tests.Test(DisabledButtonNeverActivates);
				Tests.Test(DisabledButtonBlocksItsWindow);
				Tests.Test(DisabledButtonBlocksBehind);
				Tests.Test(UnmovableWindowBlocksBehind);
				Tests.Test(BlockAtBlocksBehind);
			}
		}
		frame++;
	}

	// Control: the harness reaches an unobstructed button.
	bool ExposedButtonActivates()      => b3Focused[2].IsActive() && b3Active[2].IsJustActive() && b3Active[3].IsActive();
	bool DisabledButtonNeverActivates()=> Never(disabledActive);
	// Pinching a disabled button must not grab the window it sits on.
	bool DisabledButtonBlocksItsWindow() => Never(frontWinActive);
	bool DisabledButtonBlocksBehind()    => Never(b1Focused) && Never(b1Active);
	bool UnmovableWindowBlocksBehind()   => Never(b2Focused) && Never(b2Active);
	bool BlockAtBlocksBehind()           => Never(b4Focused) && Never(b4Active);

	static bool Never(BtnState[] states)
	{
		for (int i = 0; i < states.Length; i++)
			if (states[i].IsActive()) return false;
		return true;
	}

	// World center of a ButtonAt on the back window. Window x runs leftward.
	Vec3 ButtonCenter(Vec3 topLeft) =>
		backPose.ToMatrix().Transform(topLeft - V.XYZ(bSize.x / 2, bSize.y / 2, 0));

	// A ray straight along the view axis, so it crosses the front windows at the
	// same x/y as its target.
	static void Aim(Interactor ray, Vec3 target, BtnState pinch)
	{
		Vec3 origin = target + V.XYZ(0, 0, 0.25f);
		Vec3 dir    = V.XYZ(0, 0, -1);
		ray.Update(origin, origin + dir * 100, new Pose(origin, Quat.LookDir(dir)), origin, Vec3.Zero, pinch, BtnState.Active);
	}

	public void Shutdown()
	{
		if (Tests.IsTesting)
		{
			ray1.Destroy();
			ray2.Destroy();
			ray3.Destroy();
			ray4.Destroy();
		}
		Interaction.DefaultInteractors = oldInteractors;
	}
}
