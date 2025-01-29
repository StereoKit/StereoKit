// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2025 Nick Klingensmith
// Copyright (c) 2023-2025 Qualcomm Technologies, Inc.

using StereoKit;
using System;

class DemoUIGrabBar : ITest
{
	string title       = "UI Grab Bar";
	string description = "This is an example of using external handles to manipulate a Window's pose! Since _you_ own the Pose data, you can do whatever you want with it!\nThe grab bar below the window is a common sight to see in recent XR UI, so here's one way to replicate that with SK's API.";

	Pose grabBarPose = (Demo.contentPose * Matrix.T(0,-0.1f,0)).Pose;
	Vec2 windowSize  = Vec2.Zero; // First frame will not be in the right spot unless we know window size in advance.
	public void GrabBarWindow()
	{
		UISettings settings = UI.Settings;
		float      barSize  = Math.Max(0.01f, settings.rounding * 2);

		// Use the size of the Window to position it directly above the grab
		// bar. Note that we're using the Pose relative Up direction instead of
		// just the Y axis.
		Pose windowPose = grabBarPose;
		windowPose.position +=
			grabBarPose.Up      * (windowSize.y + barSize*0.5f + settings.gutter) +
			grabBarPose.Forward * settings.depth*0.5f;
		// Begin the window, only show the body, and don't allow the user to
		// grab it. That'd require a bit more work :)
		UI.WindowBegin("Interface", ref windowPose, new Vec2(0.15f, 0), UIWin.Body, UIMove.None);

		// Some UI to fill up space
		UI.Text("Here's a window that can be moved via a bar below the window!", TextAlign.Center);
		UI.HSeparator();
		UI.Button("Testing!");

		UI.WindowEnd();

		// Save the size of the window.
		windowSize = UI.LayoutLast.dimensions.XY;

		// Show the window grab bar at its own location.
		UI.Handle("GrabBar", ref grabBarPose, new Bounds(new Vec3(windowSize.x * 0.9f, barSize, settings.depth)), true, UIMove.FaceUser);
	}

	public void Initialize() {}
	public void Shutdown  () {}
	public void Step()
	{
		Tests.Screenshot("UIGrabBarWindow.jpg", 1, 400, 400, 90, grabBarPose.position + V.XYZ(0, 0.06f, 0.12f), grabBarPose.position + V.XYZ(0, 0.06f, 0));
		GrabBarWindow();
		Demo.ShowSummary(title, description, new Bounds(V.XY0(0, -0.04f), V.XYZ(.2f, .2f, 0)));
	}
}