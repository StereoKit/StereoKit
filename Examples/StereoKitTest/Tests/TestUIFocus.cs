using StereoKit;

// Investigates the focus/active lifecycle of UI elements. Two custom far-ray
// interactors (one per element) point at a slider and a button and pinch with a
// real Inactive -> JustActive transition. We log both each interactor's own
// focus/active state and what the UI.LastElement* queries report, to see how the
// timing lines up - and whether a button behaves any differently than a slider.
//
// Frame 0 is a warm-up: a freshly-created interactor misses its first per-frame
// reset (interaction_update runs in the UI system, before this Initialize/Step),
// so it can't focus yet. The real focus -> activate -> hold sequence runs on
// frames 1, 2, 3, matching how a startup-created interactor behaves.
class TestUIFocus : ITest
{
	float sliderVal  = 0.5f;
	Pose  windowPose = new Pose(0, 0, -0.5f, Quat.LookDir(0, 0, 1));

	DefaultInteractors oldInteractors;
	Interactor         raySlider, rayButton;
	Vec3               rayOrigin    = V.XYZ(0.05f, -0.2f, -0.35f);
	Vec3               sliderTarget, buttonTarget;
	int                frame        = 0;

	// Per-frame recording of what the UI queries reported, checked at the end.
	BtnState[] sliderFocused = new BtnState[4];
	BtnState[] sliderActive  = new BtnState[4];
	BtnState[] buttonFocused = new BtnState[4];
	BtnState[] buttonActive  = new BtnState[4];

	public void Initialize()
	{
		oldInteractors = Interaction.DefaultInteractors;
		if (!Tests.IsTesting) return; // leave the default interactors on to try by hand

		Interaction.DefaultInteractors = DefaultInteractors.None;

		// Unique sources so the two interactors never preoccupy each other.
		raySlider = Interactor.Create(InteractorType.Line, InteractorEvent.Pinch | InteractorEvent.Poke, InteractorActivation.State, InteractorSource.Unique, 0.015f, 0);
		rayButton = Interactor.Create(InteractorType.Line, InteractorEvent.Pinch | InteractorEvent.Poke, InteractorActivation.State, InteractorSource.Unique, 0.015f, 0);
		sliderTarget = windowPose.position + V.XYZ(0, -0.03f, 0); // guesses at each element, refined once focused
		buttonTarget = windowPose.position + V.XYZ(0, -0.09f, 0);

		Tests.RunForFrames(4);
	}

	public void Step()
	{
		bool scripted = Tests.IsTesting && frame < sliderFocused.Length;
		if (scripted)
		{
			// frame 0 warm-up, 1 establishes focus, 2 is the one-frame JustActive
			// that activates, 3 holds Active - same drive for both interactors.
			BtnState pinch = frame switch {
				0 => BtnState.Inactive,
				1 => BtnState.Inactive,
				2 => BtnState.Active | BtnState.JustActive,
				_ => BtnState.Active };
			Aim(raySlider, sliderTarget, pinch);
			Aim(rayButton, buttonTarget, pinch);
		}

		UI.WindowBegin("UI Focus", ref windowPose);
		UI.HSlider("Slider", ref sliderVal, 0, 1, 0.1f, 0, UIConfirm.Pinch);
		BtnState sf = UI.LastElementFocused, sa = UI.LastElementActive;
		UI.Button("Button");
		BtnState bf = UI.LastElementFocused, ba = UI.LastElementActive;
		UI.WindowEnd();

		if (scripted)
		{
			sliderFocused[frame] = sf; sliderActive[frame] = sa;
			buttonFocused[frame] = bf; buttonActive[frame] = ba;
			Log.Info($"frame {frame}:");
			Log.Info($"  slider: UI.Focused=[{sf}] UI.Active=[{sa}]  (interactor focused={raySlider.Focused != IdHash.None} active={raySlider.Active != IdHash.None})");
			Log.Info($"  button: UI.Focused=[{bf}] UI.Active=[{ba}]  (interactor focused={rayButton.Focused != IdHash.None} active={rayButton.Active != IdHash.None})");

			// Keep each ray aimed at its element once it's focused it.
			if (raySlider.TryGetFocusBounds(out Pose sp, out Bounds sb, out _)) sliderTarget = sp.ToMatrix().Transform(sb.center);
			if (rayButton.TryGetFocusBounds(out Pose bp, out Bounds bb, out _)) buttonTarget = bp.ToMatrix().Transform(bb.center);

			if (frame == 3)
			{
				Tests.Test(SliderLifecycle);
				Tests.Test(ButtonLifecycle);
				Tests.Test(ButtonMatchesSlider);
			}
		}
		frame++;
	}

	// Expected lifecycle (post warm-up): frames 0-1 report nothing, frame 2 is the
	// JustActive edge where focus and active light up together, frame 3 holds
	// Active. Focus and active line up because the warm-up removes the first-frame
	// focus gap, and focus's one-frame report lag puts it on the activation frame.
	bool SliderLifecycle() => Lifecycle(sliderFocused, sliderActive);
	bool ButtonLifecycle() => Lifecycle(buttonFocused, buttonActive);
	static bool Lifecycle(BtnState[] focused, BtnState[] active) =>
		!focused[0].IsActive() && !active[0].IsActive() &&
		!focused[1].IsActive() && !active[1].IsActive() &&
		 focused[2].IsActive() &&  focused[2].IsJustActive() &&
		 active [2].IsActive() &&  active [2].IsJustActive() &&
		 focused[3].IsActive() && !focused[3].IsJustActive() &&
		 active [3].IsActive() && !active [3].IsJustActive();

	// A button should behave identically to a slider here.
	bool ButtonMatchesSlider()
	{
		for (int i = 0; i < 4; i++)
			if (buttonFocused[i] != sliderFocused[i] || buttonActive[i] != sliderActive[i])
				return false;
		return true;
	}

	void Aim(Interactor ray, Vec3 target, BtnState pinch)
	{
		Vec3 dir = (target - rayOrigin).Normalized;
		ray.Update(rayOrigin, rayOrigin + dir * 100, new Pose(rayOrigin, Quat.LookDir(dir)), rayOrigin, Vec3.Zero, pinch, BtnState.Active);
	}

	public void Shutdown()
	{
		if (Tests.IsTesting) { raySlider.Destroy(); rayButton.Destroy(); }
		Interaction.DefaultInteractors = oldInteractors;
	}
}
