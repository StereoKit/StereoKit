using StereoKit;

// Verifies button activation CANCELLATION. A pinch/ray button fires on release -
// unless the interactor is pulled far (>15cm) from the button first, which cancels
// the press so it never fires. Under -test, two ray interactors run the same
// press/release in parallel: one stays on its button (fires), one is swung far
// away mid-press (cancels), so the cancel motion is the only difference between
// them. Outside -test the default interactors stay on, so you can try it by hand.
class TestInteractorCancel : ITest
{
	Pose windowPose = new Pose(0, 0, -0.5f, Quat.LookDir(0, 0, 1));

	DefaultInteractors oldInteractors;
	Interactor         rayClick, rayCancel;
	Vec3               rayOrigin = V.XYZ(0.05f, -0.2f, -0.35f);
	Vec3               clickTarget, cancelTarget;
	int                frame = 0;

	bool[] clickFired  = new bool[5];
	bool[] cancelFired = new bool[5];

	public void Initialize()
	{
		oldInteractors = Interaction.DefaultInteractors;
		if (!Tests.IsTesting) return; // leave the default interactors on to try by hand

		Interaction.DefaultInteractors = DefaultInteractors.None;
		// Unique sources so the two interactors never preoccupy each other.
		rayClick     = Interactor.Create(InteractorType.Line, InteractorEvent.Pinch | InteractorEvent.Poke, InteractorActivation.State, InteractorSource.Unique, 0.01f, 0);
		rayCancel    = Interactor.Create(InteractorType.Line, InteractorEvent.Pinch | InteractorEvent.Poke, InteractorActivation.State, InteractorSource.Unique, 0.01f, 0);
		clickTarget  = windowPose.position + V.XYZ(0, -0.04f, 0); // guesses at each button, refined once focused
		cancelTarget = windowPose.position + V.XYZ(0, -0.08f, 0);

		Tests.RunForFrames(5);
	}

	public void Step()
	{
		bool scripted = Tests.IsTesting && frame < clickFired.Length;
		if (scripted)
		{
			// frame 0 warm-up, 1 focus, 2 press, 3 hold, 4 release.
			BtnState pinch = frame switch {
				0 => BtnState.Inactive,
				1 => BtnState.Inactive,
				2 => BtnState.Active | BtnState.JustActive,
				3 => BtnState.Active,
				_ => BtnState.JustInactive };
			// The click ray stays on its button; the cancel ray runs the same
			// press, then on frame 3 jumps ~1m aside pointing away, so by release
			// it's well past the cancel distance.
			Aim(rayClick, rayOrigin, clickTarget, pinch);
			if (frame < 3) Aim(rayCancel, rayOrigin, cancelTarget, pinch);
			else           Aim(rayCancel, V.XYZ(1, 0, -0.35f), V.XYZ(2, 0, -0.35f), pinch);
		}

		UI.WindowBegin("Cancel", ref windowPose);
		if (!Tests.IsTesting) UI.Text("Pinch a button and pull away to cancel it", Align.Center);
		bool clickHit  = UI.Button("Click");
		bool cancelHit = UI.Button("Cancel");
		UI.WindowEnd();

		if (scripted)
		{
			clickFired [frame] = clickHit;
			cancelFired[frame] = cancelHit;
			Log.Info($"frame {frame}: click fired={clickHit}  cancel fired={cancelHit}");

			if (             rayClick .TryGetFocusBounds(out Pose cp, out Bounds cb, out _)) clickTarget  = cp.ToMatrix().Transform(cb.center);
			if (frame < 3 && rayCancel.TryGetFocusBounds(out Pose xp, out Bounds xb, out _)) cancelTarget = xp.ToMatrix().Transform(xb.center);

			if (frame == 4)
			{
				Tests.Test(ClickFires);
				Tests.Test(CancelDoesNotFire);
			}
		}
		else if (clickHit || cancelHit)
			Log.Info("Button fired!");

		frame++;
	}

	void Aim(Interactor ray, Vec3 origin, Vec3 target, BtnState pinch)
	{
		Vec3 dir = (target - origin).Normalized;
		ray.Update(origin, origin + dir * 100, new Pose(origin, Quat.LookDir(dir)), origin, Vec3.Zero, pinch, BtnState.Active);
	}

	// The control button releases on its button, so it fires exactly on frame 4.
	bool ClickFires() => clickFired[4] && !clickFired[0] && !clickFired[1] && !clickFired[2] && !clickFired[3];
	// The canceled button never fires, despite the identical press and release.
	bool CancelDoesNotFire() { for (int i = 0; i < cancelFired.Length; i++) if (cancelFired[i]) return false; return true; }

	public void Shutdown()
	{
		if (Tests.IsTesting) { rayClick.Destroy(); rayCancel.Destroy(); }
		Interaction.DefaultInteractors = oldInteractors;
	}
}
