// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2025 Nick Klingensmith
// Copyright (c) 2023-2025 Qualcomm Technologies, Inc.

using StereoKit;
using StereoKit.Framework;

class DemoPassthrough : ITest
{
	string title       = "Passthrough";
	string description = "You can set up AR with OpenXR by changing the environment blend mode! In StereoKit, this is modifiable via Device.DisplayBlend at runtime, and SKSettings.blendPreference during initialization.\n\nNote that some devices may not support each blend mode! Like a HoloLens can't be Opaque, and some VR headsets can't be transparent!";

	Pose             windowPose = Demo.contentPose.Pose;
	PassthroughFBExt passthrough;

	public void Initialize() { passthrough = SK.GetStepper<PassthroughFBExt>(); }
	public void Shutdown() { }

	public void Step()
	{
		UI.WindowBegin("Passthrough Settings", ref windowPose);

		UI.Label("Device.DisplayBlend");
		DisplayBlend currBlend = Device.DisplayBlend;

		// Radio buttons for switching between any blend modes that are
		// available on this headset!

		UI.PushEnabled(Device.ValidBlend(DisplayBlend.Opaque));
		if (UI.Radio("Opaque", currBlend == DisplayBlend.Opaque))
			Device.DisplayBlend = DisplayBlend.Opaque;
		UI.PopEnabled();

		UI.PushEnabled(Device.ValidBlend(DisplayBlend.Blend));
		if (UI.Radio("Blend", currBlend == DisplayBlend.Blend))
			Device.DisplayBlend = DisplayBlend.Blend;
		UI.PopEnabled();

		UI.PushEnabled(Device.ValidBlend(DisplayBlend.Additive));
		if (UI.Radio("Additive", currBlend == DisplayBlend.Additive))
			Device.DisplayBlend = DisplayBlend.Additive;
		UI.PopEnabled();

		// If we have the Facebook Passthrough extension turned on, show some
		// options for that as well! This extension has a few additional
		// options that goes beyond the simple blend mode.
		if (passthrough != null)
		{
			UI.HSeparator();
			bool toggle = passthrough.Enabled;
			UI.Label(passthrough.Available
				? "FB Passthrough EXT available!"
				: "No FB passthrough EXT available :(");
			UI.PushEnabled(passthrough.Available);
			if (UI.Toggle("Passthrough", ref toggle))
				passthrough.Enabled = toggle;
			UI.PopEnabled();
		}

		UI.WindowEnd();

		Demo.ShowSummary(title, description,
			new Bounds(V.XY0(0, -0.03f), new Vec3(.3f, .15f, 0)));
	}
}