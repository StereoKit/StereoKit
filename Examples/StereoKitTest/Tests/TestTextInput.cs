using StereoKit;

// Verifies UI.Input's two input channels. Text arrives through the text queue,
// and editing intent (backspace, delete, enter) arrives as key presses, which is
// the only route a hardware keyboard has on Wayland, Web, and Android. Injected
// keys and text land at the start of the following frame, so each scripted frame
// asserts what the previous frame injected.
class TestTextInput : ITest
{
	Pose windowPose = new Pose(0, 0, -0.5f, Quat.LookDir(0, 0, 1));

	DefaultInteractors oldInteractors;
	Interactor         ray;
	Vec3               rayOrigin = V.XYZ(0.05f, -0.2f, -0.35f);
	Vec3               target;
	int                frame = 0;
	string             text  = "";

	string afterType, afterBackspace, afterDelete, afterInjectedControl;
	bool   submitted;

	public void Initialize()
	{
		oldInteractors = Interaction.DefaultInteractors;
		if (!Tests.IsTesting) return; // leave the default interactors on to try by hand

		Interaction.DefaultInteractors = DefaultInteractors.None;
		ray    = Interactor.Create(InteractorType.Line, InteractorEvent.Pinch | InteractorEvent.Poke, InteractorActivation.State, InteractorSource.Unique, 0.01f, 0);
		target = windowPose.position + V.XYZ(0, -0.04f, 0); // a guess at the field, refined once focused

		Tests.RunForFrames(11);
	}

	public void Step()
	{
		bool scripted = Tests.IsTesting && frame < 11;
		if (scripted)
		{
			// Frames 0-2 click the field to focus it: hover, press, release.
			BtnState pinch = frame switch {
				0 => BtnState.Inactive,
				1 => BtnState.Active | BtnState.JustActive,
				2 => BtnState.JustInactive,
				_ => BtnState.Inactive };
			Vec3 dir = (target - rayOrigin).Normalized;
			ray.Update(rayOrigin, rayOrigin + dir * 100, new Pose(rayOrigin, Quat.LookDir(dir)), rayOrigin, Vec3.Zero, pinch, BtnState.Active);
		}

		UI.WindowBegin("Text Input", ref windowPose);
		if (!Tests.IsTesting) UI.Text("Type, then try backspace, delete, and enter", Align.Center);
		bool changed = UI.Input("field", ref text, new Vec2(0.2f, 0));
		UI.WindowEnd();

		if (scripted)
		{
			// Read what the previous frame's injection produced, then set up the
			// next one.
			switch (frame)
			{
				case 2: Input.TextInject("abc"); break;
				case 3: afterType      = text; Input.KeyInjectPress  (Key.Backspace); break;
				case 4: afterBackspace = text; Input.KeyInjectRelease(Key.Backspace); Input.KeyInjectPress(Key.Left); break;
				case 5:                        Input.KeyInjectRelease(Key.Left);      Input.KeyInjectPress(Key.Del);  break;
				case 6: afterDelete    = text; Input.KeyInjectRelease(Key.Del); break;
				// A control code injected as text is not text, so it must not
				// delete and must not land in the buffer.
				case 7: Input.TextInject("\b"); break;
				case 8: afterInjectedControl = text; Input.KeyInjectPress(Key.Return); break;
				case 9: submitted = changed;   Input.KeyInjectRelease(Key.Return); break;
			}
			Log.Info($"frame {frame}: text=\"{text}\" changed={changed}");

			if (frame < 2 && ray.TryGetFocusBounds(out Pose p, out Bounds b, out _))
				target = p.ToMatrix().Transform(b.center);

			if (frame == 10)
			{
				Tests.Test(TextInserts);
				Tests.Test(BackspaceDeletesBack);
				Tests.Test(DeleteDeletesForward);
				Tests.Test(InjectedControlCodeIgnored);
				Tests.Test(EnterSubmits);
			}
		}

		frame++;
	}

	bool TextInserts()               => afterType      == "abc";
	// Backspace at the end of the string removes the last character.
	bool BackspaceDeletesBack()      => afterBackspace == "ab";
	// Left moved the caret between 'a' and 'b', so Delete removes the 'b'.
	bool DeleteDeletesForward()      => afterDelete    == "a";
	bool InjectedControlCodeIgnored()=> afterInjectedControl == "a";
	// Enter reports a change and closes the field out.
	bool EnterSubmits()              => submitted;

	public void Shutdown()
	{
		if (Tests.IsTesting) ray.Destroy();
		Interaction.DefaultInteractors = oldInteractors;
	}
}
