// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKit;
using System;

class DemoRenderScaling : ITest
{
	string title       = "Render Scaling";
	string description = "Sometimes you need to boost the number of pixels your app renders, to reduce jaggies! Renderer.Scaling and Renderer.Multisample let you increase the size of the draw surface, and multisample each pixel. Renderer.ViewportScaling shrinks the drawn area instead, without reallocating anything. AppWindow.Main.RequestFullscreen changes the window's size, and the render surface follows it.\n\nThis is powerful stuff, so use it sparingly!";

	Pose windowPose = Demo.contentPose.Pose;

	float multisample;
	float scaling;
	float viewScaling;

	public void Initialize()
	{
		multisample = Renderer.Multisample;
		scaling     = Renderer.Scaling;
		viewScaling = Renderer.ViewportScaling;
	}
	public void Shutdown() { }
	public void Step()
	{
		UI.WindowBegin("Render Scaling Settings", ref windowPose, new Vec2(0.3f,0));

		UI.Label("Swapchain scaling");
		UI.Label($"{scaling:0.00}", V.XY(0.04f,0));
		UI.SameLine();
		UI.HSlider("scaling", ref scaling, 0.1f, 2, 0.05f);

		UI.HSeparator();

		UI.Label("MSAA");
		UI.Label($"{(int)multisample}", V.XY(0.04f, 0));
		UI.SameLine();
		UI.HSlider("msaa", ref multisample, 1, 8, 1);

		if (Input.Key(Key.Right).IsJustActive()) scaling = Math.Min(scaling + 0.1f, 2);
		if (Input.Key(Key.Left ).IsJustActive()) scaling = Math.Max(scaling - 0.1f, 0.1f);
		if (Input.Key(Key.Up   ).IsJustActive()) multisample = Math.Min(multisample + 1, 8);
		if (Input.Key(Key.Down ).IsJustActive()) multisample = Math.Max(multisample - 1, 1);

		if (UI.Button("Confirm") || Input.Key(Key.Space).IsJustActive())
		{
			// Read back what we set, since these get clamped to whatever the
			// GPU actually supports.
			Renderer.Multisample = (int)multisample;
			multisample          = Renderer.Multisample;
			Renderer.Scaling     = scaling;
			scaling              = Renderer.Scaling;
		}

		UI.HSeparator();

		UI.Label("Viewport Scaling");
		UI.Label($"{viewScaling:0.00}", V.XY(0.04f, 0));
		UI.SameLine();
		if (UI.HSlider("viewscaling", ref viewScaling, 0.1f, 1, 0))
			Renderer.ViewportScaling = viewScaling;

		UI.HSeparator();

		// Fullscreen only means something when there's a desktop window, and
		// even there the window manager gets the final say. Reading the state
		// back every frame means the toggle shows what actually happened,
		// rather than what we asked for.
		AppWindow window     = AppWindow.Main;
		bool      fullscreen = window != null && window.Fullscreen;
		UI.PushEnabled(window != null);
		if (UI.Toggle("Fullscreen", ref fullscreen))
			window.RequestFullscreen(fullscreen);
		UI.PopEnabled();

		UI.WindowEnd();

		Demo.ShowSummary(title, description, new Bounds(V.XY0(0, -0.16f), V.XYZ(.34f, .42f, 0.1f)));
	}

}