// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKit;

class DemoDevice : ITest
{
	string title       = "Device";
	string description = "The Device class contains a number of interesting bits of data about the device it's running on! Most of this is just information, but there's a few properties that can also be modified.";

	Pose windowPose = Demo.contentPose.Pose;

	public void Initialize() { }
	public void Shutdown  () { }

	public void Step() {
		UI.WindowBegin("Device Info", ref windowPose);

		Vec2 labelSize = V.XY(0.08f,0);
		UI.Label("Name", labelSize); UI.SameLine();
		UI.Label(Device.Name, false);
		UI.Label("Runtime", labelSize); UI.SameLine();
		UI.Label(Device.Runtime, false);
		UI.Label("Version", labelSize); UI.SameLine();
		ulong v = Device.RuntimeVersion;
		UI.Label($"{(v >> 48) & 0xFFFF}.{(v >> 32) & 0xFFFF}.{v & 0xFFFFFFFF}", false);
		UI.Label("GPU", labelSize); UI.SameLine();
		UI.Label(Device.GPU, false);
		UI.Label("Tracking", labelSize); UI.SameLine();
		UI.Label(Device.Tracking.ToString(), false);
		UI.Label("Display", labelSize); UI.SameLine();
		UI.Label(Device.DisplayType.ToString(), false);
		UI.Label("Blend", labelSize); UI.SameLine();
		UI.Label(Device.DisplayBlend.ToString(), false);

		UI.HSeparator();

		labelSize = V.XY(0.14f, 0);
		UI.Label("Has Eye Gaze", labelSize); UI.SameLine();
		UI.Label(Device.HasEyeGaze.ToString(), false);
		UI.Label("Has Hand Tracking", labelSize); UI.SameLine();
		UI.Label(Device.HasHandTracking.ToString(), false);

		UI.WindowEnd();
		Demo.ShowSummary(title, description, new Bounds(V.XY0(0, -0.15f), V.XYZ(.34f, .4f, .1f)));
	}
}