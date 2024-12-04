// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKit;

class DemoUI : ITest
{
	string title       = "UI";
	string description = "...";

	Pose  windowPose = new Pose(-.2f, 0, -0.6f, Quat.LookDir(0,0,1));

	bool  showHeader = true;
	float slider     = 0.5f;

	Sprite powerSprite = Sprite.FromFile("power.png", SpriteType.Single);

	Model  clipboard     = Model.FromFile("Clipboard.glb", Default.ShaderUI);
	Sprite logoSprite    = Sprite.FromFile("StereoKitWide.png", SpriteType.Single);
	Pose   clipboardPose = new Pose(0.2f, 0, -0.6f, Quat.LookDir(0,0,1));
	bool   clipToggle;
	float  clipSlider;
	int    clipOption = 1;

	public void Step()
	{
		UI.WindowBegin("Window", ref windowPose, new Vec2(20, 0) * U.cm, showHeader?UIWin.Normal:UIWin.Body);

		UI.Toggle("Show Header", ref showHeader);

		UI.Label("Slide");
		UI.SameLine();
		UI.HSlider("slider", ref slider, 0, 1, 0.2f, 72 * U.mm);

		if (UI.ButtonImg("Exit", powerSprite))
			SK.Quit();

		UI.WindowEnd();


		UI.HandleBegin("Clip", ref clipboardPose, clipboard.Bounds);
		clipboard.Draw(Matrix.Identity);

		UI.LayoutArea(new Vec3(13, 15, 0) * U.cm, new Vec2(26, 30) * U.cm);

		UI.Image(logoSprite, new Vec2(22,0) * U.cm);

		UI.Toggle("Toggle", ref clipToggle);
		UI.HSlider("Slide", ref clipSlider, 0, 1, 0);

		if (UI.Radio("Opt1", clipOption == 1)) clipOption = 1;
		UI.SameLine();
		if (UI.Radio("Opt2", clipOption == 2)) clipOption = 2;
		UI.SameLine();
		if (UI.Radio("Opt3", clipOption == 3)) clipOption = 3;

		UI.HandleEnd();

		Demo.ShowSummary(title, description, new Bounds(V.XY0(-.02f,0.1f), V.XYZ(.7f, .46f, 0.1f)));
	}

	public void Initialize() {
		Tests.RunForFrames(2);
	}
	public void Shutdown() { }
}