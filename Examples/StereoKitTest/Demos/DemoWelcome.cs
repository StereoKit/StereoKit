// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKit;

class DemoWelcome : ITest
{
	Model  logo;
	string message = "Welcome fellow developer!\n\nThis is the StereoKit test app, a collection of demos and tests that cover StereoKit's major features. Use the panel to your left to navigate around the app, and enjoy!\n\nCheck your hand menu, (or '~' for keyboard users) for some debugging tools :)";

	public void Initialize()
	{
		logo = Model.FromFile("StereoKit.glb");
	}

	public void Step()
	{
		Hierarchy.Push(Demo.contentPose);
		float scale = 1.3f;
		Vec3  size  = logo.Bounds.dimensions * scale;
		logo.Draw(Matrix.TRS(V.XYZ(0, size.y/2.0f + 0.05f, 0), Quat.Identity, scale));
		Text.Add(message, Matrix.S(1.25f), V.XY(.6f, 0), TextFit.Wrap, TextAlign.TopCenter, TextAlign.TopLeft);
		Hierarchy.Pop();

		Demo.ShowSummary("", "", new Bounds(new Vec3(0,-0.05f,0), new Vec3(1, .7f, 0.1f)));
	}

	public void Shutdown()
	{
	}
}