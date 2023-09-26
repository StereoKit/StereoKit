// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKit;
using StereoKit.Framework;

class DemoSkeleton : ITest
{
	string title       = "Skeleton Estimation";
	string description = "With knowledge about where the head and hands are, you can make a decent guess about where some other parts of the body are! The StereoKit repository contains an AvatarSkeleton IStepper to show a basic example of how something like this can be done.";

	AvatarSkeleton avatar;
	public void Initialize()
	{
		avatar = SK.AddStepper<AvatarSkeleton>();
	}

	public void Step()
	{
		Demo.ShowSummary(title, description, new Bounds(.2f, .2f, 0.1f));
	}

	public void Shutdown()
	{ 
		SK.RemoveStepper(avatar);
	}
}