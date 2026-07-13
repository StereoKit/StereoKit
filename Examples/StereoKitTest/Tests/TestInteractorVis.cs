// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith
// Copyright (c) 2026 Qualcomm Technologies, Inc.

using System;
using StereoKit;

/// This test turns off StereoKit's built-in interactor visualization
/// (`Interaction.DefaultDraw`) and attempts to replicate the internal far-ray
/// indicator (`interactor_show_ray` in interactor_modes.cpp) entirely at the
/// application level, using only the public `Interactor` API.
///
/// It drives a single custom Line interactor with simulated motion so the ray
/// focuses and activates a UI button, exercising the "centered" / curved ray
/// behavior the internal indicator produces.
class TestInteractorVis : ITest
{
	DefaultInteractors prevDefault;
	bool               prevDraw;
	Interactor         ray;

	// Per-interactor smoothing state. Internally this lives alongside each
	// default interactor (interact_mode_hands_t::ray_visible / ray_active);
	// here we keep it ourselves since we own the interactor.
	float rayVisible = 0;
	float rayActive  = 0;

	Pose windowPose = new Pose(0, 0, -0.5f, Quat.LookDir(-Vec3.Forward));

	// Origin of the ray, roughly where a hand's aim ray would start.
	Vec3 rayOrigin = V.XYZ(0, -0.15f, 0.25f);

	public void Initialize()
	{
		// Replace SK's default interactors with our own, and turn off the
		// built-in ray visualization so we can draw our own.
		prevDefault = Interaction.DefaultInteractors;
		prevDraw    = Interaction.DefaultDraw;
		Interaction.DefaultInteractors = DefaultInteractors.None;
		Interaction.DefaultDraw        = false;

		ray = Interactor.Create(InteractorType.Line, InteractorEvent.Pinch | InteractorEvent.Poke, InteractorActivation.State, InteractorSource.Max, 0.05f, 2);

		// The creation parameters are now readable back off the interactor.
		Tests.Test(CreationParamsReadBack);

		Tests.RunForFrames(20);
	}

	// Verifies the creation parameters can be read back off the Interactor
	// exactly as they were passed to Interactor.Create above.
	bool CreationParamsReadBack() =>
		ray.Type          == InteractorType.Line                          &&
		ray.Events        == (InteractorEvent.Pinch|InteractorEvent.Poke) &&
		ray.Activation    == InteractorActivation.State                   &&
		ray.Source        == InteractorSource.Max                         &&
		ray.SecondaryDims == 2;

	public void Shutdown()
	{
		ray.Destroy();
		Interaction.DefaultInteractors = prevDefault;
		Interaction.DefaultDraw        = prevDraw;
	}

	Vec3 target;
	public void Step()
	{
		// Aim the ray straight at the button so it stays focused, and report it
		// as active so we exercise the active/curved branch of the indicator.
		Vec3     dir    = (target - rayOrigin).Normalized;
		BtnState active = BtnState.Active;
		ray.Update(rayOrigin, rayOrigin + dir * 100, new Pose(rayOrigin, Quat.LookDir(dir)), rayOrigin, Vec3.Zero, active, BtnState.Active);

		// Draw the far-ray indicator (a shared helper on Tests, ported from the
		// internal interactor_show_ray).
		Tests.DrawInteractorRay(ray, ref rayVisible, ref rayActive);

		UI.WindowBegin("Interactor Vis", ref windowPose, V.XY(0.2f, 0));
		UI.Button("Target");
		target = Hierarchy.ToWorld(UI.LayoutLast.center - V.XYZ(UI.LayoutLast.dimensions.x/2 + ray.Radius*0.8f,0,0));
		UI.WindowEnd();

		// Screenshot late enough that the visibility/active smoothing has
		// ramped up to full.
		Tests.Screenshot("Tests/InteractorVis.jpg", 18, 600, 400, 60, V.XYZ(0.25f, 0.1f, 0.4f), V.XYZ(0, 0, -0.5f));
	}
}
