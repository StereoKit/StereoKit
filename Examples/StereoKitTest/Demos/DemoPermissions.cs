// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2025 Nick Klingensmith
// Copyright (c) 2025 Qualcomm Technologies, Inc.

using StereoKit;

class DemoPermissions : ITest
{
	string title       = "Permissions";
	string description = "StereoKit comes with APIs for managing cross-platform permissions. While on Desktop, permissions are almost not an issue, platforms like Android can get kinda complicated! Here, StereoKit provides an API that handles the more complicated permission issues of platforms like Android, and also can be a regular part of your desktop code!";

	Pose windowPose = Demo.contentPose.Pose;

	TextStyle warn;
	TextStyle good;

	public void Initialize()
	{
		warn = TextStyle.FromFont(Font.Default, UI.TextStyle.LayoutHeight, Color.HSV(0.16f, 0.7f, 0.9f));
		good = TextStyle.FromFont(Font.Default, UI.TextStyle.LayoutHeight, Color.HSV(0.33f, 0.7f, 0.9f));
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
		UI.WindowBegin("Permissions", ref windowPose);
		for (int i = 0; i < (int)PermissionType.Max; i++)
		{
			PermissionType permission = (PermissionType)i;

			UI.PushId(i);

			// Show the name of the permission we're working with this line
			UI.Label(permission.ToString(), new Vec2(0.1f, 0));
			UI.SameLine();

			// Display the permission's current state
			PermissionState state = Permission.State(permission);
			switch (state)
			{
				case PermissionState.Granted:     UI.PushTextStyle(good); break;
				case PermissionState.Unavailable: UI.PushTextStyle(warn); break;
				case PermissionState.Unknown:     UI.PushTextStyle(warn); break;
				default: UI.PushTextStyle(UI.TextStyle); break;
			}
			UI.Label(state.ToString(), new Vec2(0.1f, 0));
			UI.PopTextStyle();
			UI.SameLine();

			// Allow the user to request the permission if it needs it
			UI.PushEnabled(state == PermissionState.Capable && Permission.IsInteractive(permission));
			if (UI.Button("Request")) Permission.Request(permission);
			UI.PopEnabled();

			UI.PopId();
		}
		UI.WindowEnd();

		Demo.ShowSummary(title, description,
			new Bounds(UI.LayoutLast.center, UI.LayoutLast.dimensions * 1.2f));
	}
}