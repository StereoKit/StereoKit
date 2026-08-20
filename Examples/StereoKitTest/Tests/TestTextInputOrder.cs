using StereoKit;

// Verifies the three UI.Input behaviors that only appear when several events
// land in the same frame, which is what a frame hitch produces. Each of these
// fails under the older drain-text-then-poll-keys approach.
class TestTextInputOrder : ITest
{
	Pose windowPose = new Pose(0, 0, -0.5f, Quat.LookDir(0, 0, 1));

	DefaultInteractors oldInteractors;
	Interactor         ray;
	Vec3               rayOrigin = V.XYZ(0.05f, -0.2f, -0.35f);
	Vec3               target;
	int                frame = 0;
	string             text  = "";

	string afterSeed, afterInterleave, afterRepeat, afterShiftEnter;

	public void Initialize()
	{
		oldInteractors = Interaction.DefaultInteractors;
		if (!Tests.IsTesting) return;

		Interaction.DefaultInteractors = DefaultInteractors.None;
		ray    = Interactor.Create(InteractorType.Line, InteractorEvent.Pinch | InteractorEvent.Poke, InteractorActivation.State, InteractorSource.Unique, 0.01f, 0);
		target = windowPose.position + V.XYZ(0, -0.04f, 0);

		Tests.RunForFrames(10);
	}

	public void Step()
	{
		bool scripted = Tests.IsTesting && frame < 10;
		if (scripted)
		{
			BtnState pinch = frame switch {
				0 => BtnState.Inactive,
				1 => BtnState.Active | BtnState.JustActive,
				2 => BtnState.JustInactive,
				_ => BtnState.Inactive };
			Vec3 dir = (target - rayOrigin).Normalized;
			ray.Update(rayOrigin, rayOrigin + dir * 100, new Pose(rayOrigin, Quat.LookDir(dir)), rayOrigin, Vec3.Zero, pinch, BtnState.Active);
		}

		UI.WindowBegin("Text Order", ref windowPose);
		if (!Tests.IsTesting) UI.Text("Scripted same-frame input tests", Align.Center);
		UI.Input("field", ref text, new Vec2(0.2f, 0));
		UI.WindowEnd();

		if (scripted)
		{
			switch (frame)
			{
				case 2: Input.TextInject("abc"); break;
				// Type, delete, type, all inside one frame. Applied in order
				// this is "abcy"; applied text-first it would be "abcx".
				case 3:
					afterSeed = text;
					Input.TextInject      ("x");
					Input.KeyInjectPress  (Key.Backspace);
					Input.TextInject      ("y");
					Input.KeyInjectRelease(Key.Backspace);
					break;
				// Two backspaces in one frame must delete two characters.
				case 4:
					afterInterleave = text;
					Input.KeyInjectPress  (Key.Backspace);
					Input.KeyInjectRelease(Key.Backspace);
					Input.KeyInjectPress  (Key.Backspace);
					Input.KeyInjectRelease(Key.Backspace);
					break;
				// Shift pressed and released around Return in one frame. Only
				// the per-event modifier snapshot gets this right.
				case 5:
					afterRepeat = text;
					Input.KeyInjectPress  (Key.Shift);
					Input.KeyInjectPress  (Key.Return);
					Input.KeyInjectRelease(Key.Return);
					Input.KeyInjectRelease(Key.Shift);
					break;
				case 6: afterShiftEnter = text; break;
				case 8:
					Tests.Test(Seeded);
					Tests.Test(InterleaveInOrder);
					Tests.Test(SameFrameRepeatDeletesTwice);
					Tests.Test(SameFrameShiftEnterAddsLine);
					break;
			}
			Log.Info($"frame {frame}: text=\"{text.Replace("\n", "\\n")}\"");

			if (frame < 2 && ray.TryGetFocusBounds(out Pose p, out Bounds b, out _))
				target = p.ToMatrix().Transform(b.center);
		}

		frame++;
	}

	bool Seeded()                       => afterSeed       == "abc";
	bool InterleaveInOrder()            => afterInterleave == "abcy";
	bool SameFrameRepeatDeletesTwice()  => afterRepeat     == "ab";
	// Shift+Enter adds a line rather than submitting, so the field stays open.
	bool SameFrameShiftEnterAddsLine()  => afterShiftEnter == "ab\n";

	public void Shutdown()
	{
		if (Tests.IsTesting) ray.Destroy();
		Interaction.DefaultInteractors = oldInteractors;
	}
}
