// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith
// Copyright (c) 2026 Qualcomm Technologies, Inc.

using System;
using StereoKit;

// Interactive-only scene (no pass/fail) for poking at StereoKit's default
// interactor modes on real devices. Pick a DefaultInteractors mode with the
// radios, then exercise the assorted UI elements with whatever input the
// device provides (hands, controllers, mouse). The status readout shows how
// many interactors the current mode actually produced, plus their tracking
// and focus state, which is handy for diagnosing "why can't I touch this?"
class TestInteractorModes : ITest
{
	DefaultInteractors prevMode;
	bool               prevDraw;

	Pose modePose  = new Pose(-0.2f, 0.1f, -0.5f, Quat.LookDir(0, 0, 1));
	Pose inputPose = new Pose( 0.2f, 0.1f, -0.5f, Quat.LookDir(0, 0, 1));

	// State for the various test input elements.
	bool   toggle;
	int    radio   = 0;
	float  hSlider = 0.5f;
	float  vSlider = 0.5f;
	string text    = "Edit me";
	string number  = "42";

	public void Initialize()
	{
		// Preserve global interaction state so we don't leak settings into
		// other tests when running headlessly.
		prevMode = Interaction.DefaultInteractors;
		prevDraw = Interaction.DefaultDraw;
	}

	public void Shutdown()
	{
		Interaction.DefaultInteractors = prevMode;
		Interaction.DefaultDraw        = prevDraw;
	}

	public void Step()
	{
		ShowModeWindow ();
		ShowInputWindow();
	}

	void ShowModeWindow()
	{
		UI.WindowBegin("Interactor Mode", ref modePose, V.XY(0.22f, 0));

		// One radio per DefaultInteractors mode. Selecting one applies it
		// immediately so the change is visible on the next frame. Modes that
		// would leave the user with no working interactor (and thus no way to
		// switch back) are shown but grayed out.
		DefaultInteractors current = Interaction.DefaultInteractors;
		foreach (DefaultInteractors mode in Enum.GetValues(typeof(DefaultInteractors)))
		{
			UI.PushEnabled(ModeAvailable(mode));
			if (UI.Radio(mode.ToString(), mode == current))
				Interaction.DefaultInteractors = mode;
			UI.PopEnabled();
		}

		UI.HSeparator();

		// Toggle the built-in ray/indicator drawing so it's easy to see what
		// each mode renders.
		bool draw = Interaction.DefaultDraw;
		if (UI.Toggle("Draw Indicators", ref draw))
			Interaction.DefaultDraw = draw;

		UI.HSeparator();

		// Live readout of the interactors the current mode produced. Empty
		// means the mode (or device) isn't providing any.
		UI.Label("Active interactors: ");
		foreach (Interactor actor in Interactor.All)
		{
			bool tracked = actor.Tracked.IsActive();
			bool focused = actor.Focused != IdHash.None;
			UI.Label($"{actor.Source} {actor.Type}: {(tracked ? "tracked" : "lost")}{(focused ? ", focusing" : "")}");
		}

		UI.WindowEnd();
	}

	void ShowInputWindow()
	{
		UI.WindowBegin("Test Inputs", ref inputPose, V.XY(0.28f, 0));

		// Buttons of a few flavors.
		if (UI.Button("Button")) {}
		UI.SameLine();
		if (UI.ButtonImg("Image", Sprite.Shift)) {}
		UI.SameLine();
		if (UI.ButtonRound("Round", Sprite.Close)) {}

		UI.HSeparator();

		// Toggle + a small radio group.
		UI.Toggle("Toggle", ref toggle);
		if (UI.Radio("Opt A", radio == 0)) radio = 0;
		UI.SameLine();
		if (UI.Radio("Opt B", radio == 1)) radio = 1;
		UI.SameLine();
		if (UI.Radio("Opt C", radio == 2)) radio = 2;

		UI.HSeparator();

		// Sliders, both pinch- and poke-driven.
		UI.HSlider("HSlider", ref hSlider, 0, 1, 0, 0,     UIConfirm.Pinch);
		UI.VSlider("VSlider", ref vSlider, 0, 1, 0, 0.08f, UIConfirm.Push);

		UI.HSeparator();

		// Text input, including a numeric keyboard variant.
		UI.Input("Text",   ref text);
		UI.Input("Number", ref number, default, TextContext.Number);

		UI.WindowEnd();
	}

	// Would selecting this mode leave the user with a working interactor on
	// the current backend? A radio is only enabled if it would, otherwise
	// picking it could trap the app with nothing to click to switch back.
	// We key off the resolved SK.Settings.mode, which gives us the input each
	// backend actually provides:
	//   XR        -> hands and/or controllers (no mouse)
	//   Simulator -> mouse + mouse-driven simulated hands (no controllers)
	//   Window    -> mouse only (no XR input simulated)
	// 'Default' always maps to a backend default that works, so it stays
	// enabled as a guaranteed escape hatch. 'None' has no interactors by
	// definition, so it's always grayed out.
	static bool ModeAvailable(DefaultInteractors mode) => mode switch
	{
		DefaultInteractors.Default     => true,
		DefaultInteractors.None        => false,
		DefaultInteractors.Mouse       => SK.Settings.mode is AppMode.Simulator or AppMode.Window,
		DefaultInteractors.Hands       => SK.Settings.mode is AppMode.XR or AppMode.Simulator,
		// Controllers need real tracked controllers; the simulator's
		// mouse-driven hands report as 'simulated', which 'All' also routes to
		// controllers, so both are XR-only.
		DefaultInteractors.Controllers => SK.Settings.mode is AppMode.XR,
		DefaultInteractors.All         => SK.Settings.mode is AppMode.XR,
		_                              => true,
	};
}
