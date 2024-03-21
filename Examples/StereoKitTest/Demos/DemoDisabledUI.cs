// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

using StereoKit;

class DemoDisabledUI : ITest
{
	string title       = "Disabled UI Elements";
	string description = "UI elements are interactive and usually have actions associated with them. Some usecase requires elements to be disabled and Stereokit supports disabled elements and doesn't trigger the corresponding action associated with the elements on interaction.";
	Sprite sprSearch = Sprite.FromFile("search.png");
	Sprite sprToggleOn = Sprite.FromFile("mic_icon.png");
	Sprite sprToggleOff = Sprite.FromFile("power.png");
	bool[] disabledToggles = new bool[5];
	int disabledRadio = 0;

	Pose windowPose = Demo.contentPose.Pose;

	Sprite powerSprite = Sprite.FromFile("power.png", SpriteType.Single);

	public void Step()
	{

		UI.WindowBegin("Window", ref windowPose, new Vec2(40, 0) * U.cm);
		///
		/// PushEnabled method takes a boolean that defines if the elements 
		/// following it need to enabled or disabled.
		/// 
		UI.PushEnabled(false);
		/// 
		/// Here's that button that is disabled! Disabled button cannot be
		/// acted upon and doesn't execute code attached to action performed on it.
		/// 
		if (UI.Button("UI.Button"))
		{
			Log.Info("Button was pressed.");
		}
		/// 
		/// Here's that button with an image that is disabled! Disabled button cannot be
		/// acted upon and doesn't execute code attached to action performed on it.
		/// 
		if (UI.ButtonImg("UI.ButtonImg", sprSearch, UIBtnLayout.Left))
		{
			Log.Info("ButtonImg was pressed.");
		}
		/// 
		/// Here's that button with an image without text that is disabled! Disabled button cannot be
		/// acted upon and doesn't execute code attached to action performed on it.
		/// 
		if (UI.ButtonImg("UI.ButtonImgNoText", sprSearch, UIBtnLayout.CenterNoText))
		{
			Log.Info("ButtonImgNoText was pressed.");
		}
		///
		/// Here's a label that is disabled.
		///
		UI.Label("DisabledLabel");
		///
		/// Here's couple of Toggle elements that are disabled.
		///
		UI.Toggle("UI.Toggle", ref disabledToggles[0]);
		UI.SameLine();
		UI.Toggle("UI.CustomImageToggle", ref disabledToggles[2], sprToggleOff, sprToggleOn);
		///
		/// Here's a Radio element that is disabled.
		///
		if (UI.Radio("UI.DisRadio", disabledRadio == 0)) { 
			disabledRadio = 0; 
		}
		UI.SameLine();
		if (UI.Radio("UI.DisRadio1", disabledRadio == 1)) {
			disabledRadio = 1;
		}
		UI.SameLine();
		if (UI.Radio("UI.DisRadio2", disabledRadio == 2)) {
			disabledRadio = 2;
		}
		///
		/// PopEnabled removes the disabling affect for the elements that follow.
		/// 
		UI.PopEnabled();
		///
		/// Here's how you use a simple button! that lets user to quit the application.
		/// Button is not disabled as PopEnabled was executed in previous line.
		/// 
		if (UI.ButtonImg("Exit", powerSprite))
		{
			SK.Quit();
		}

		/// 
		/// And for every begin, there must also be an end! StereoKit will
		/// log errors when this occurs, so keep your eyes peeled for that!
		/// 
		UI.WindowEnd();

		Demo.ShowSummary(title, description, new Bounds(V.XY0(0, -0.125f), new Vec3(.1f, .4f, 0)));
	}

	public void Initialize() {
		Tests.RunForFrames(2);
	}
	public void Shutdown() { }
}