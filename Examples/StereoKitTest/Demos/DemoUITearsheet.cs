// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKit;
using System;

class DemoUITearsheet : ITest
{
	string title       = "UI Tearsheet";
	string description = "An enumeration of all the different types of UI elements!";

	Sprite sprToggleOn  = Sprite.FromFile("mic_icon.png");
	Sprite sprToggleOff = Sprite.FromFile("power.png");
	Sprite sprSearch    = Sprite.FromFile("search.png");

	public void Initialize()
	{
		Tests.RunForFrames(2);
	}

	public void Shutdown()
	{
	}

	int index = 1;
	void Unique(Action a) { UI.PushId(index); index += 1; a(); UI.PopId(); }

	Pose   buttonWindowPose = new Pose(0.45f,0,0);
	bool[] toggles          = new bool[10];
	int    radio            = 0;
	void ShowButtonWindow()
	{
		UI.WindowBegin("Buttons", ref buttonWindowPose);

		Unique(() => UI.Button("UI.Button"));
		UI.SameLine();
		Unique(() => UI.Button("UI.Button", new Vec2(0.14f,0)));

		Unique(() => UI.ButtonImg("UI.ButtonImg", sprSearch, UIBtnLayout.Left));
		UI.SameLine();
		Unique(() => UI.ButtonImg("UI.ButtonImg", sprSearch, UIBtnLayout.Center));

		Unique(() => UI.ButtonImg("UI.ButtonImg", sprSearch, UIBtnLayout.Right));
		UI.SameLine();
		Unique(() => UI.ButtonImg("UI.ButtonImg", sprSearch, UIBtnLayout.CenterNoText));

		Unique(() => UI.ButtonImg("UI.ButtonImg", sprSearch, UIBtnLayout.None));

		Unique(() => UI.ButtonImg("UI.ButtonImg", sprSearch, new Color(1, 0, 0, 1), UIBtnLayout.CenterNoText));
		UI.SameLine();
		Unique(() => UI.ButtonImg("UI.ButtonImgTint", sprSearch, new Color(1, 0, 0, 1)));

		UI.Label("UI.ButtonRound");
		UI.SameLine();
		Unique(() => UI.ButtonRound("UI.ButtonRound", sprSearch));

		Unique(() => UI.Toggle("UI.Toggle", ref toggles[0]));
		UI.SameLine();
		Unique(() => UI.Toggle("UI.Toggle", ref toggles[1], new Vec2(0.14f,0)));

		Unique(() => UI.Toggle("UI.Toggle", ref toggles[2], sprToggleOff, sprToggleOn));
		UI.SameLine();
		Unique(() => UI.Toggle("UI.Toggle", ref toggles[3], sprToggleOff, sprToggleOn, UIBtnLayout.Left, new Vec2(0.14f, 0)));

		Unique(() => UI.Toggle("UI.Toggle", ref toggles[4], null, null));

		Unique(() => { if (UI.Radio("UI.Radio", radio == 0)) radio = 0; });
		UI.SameLine();
		Unique(() => { if (UI.Radio("UI.Radio", radio == 1)) radio = 1; });
		UI.SameLine();
		Unique(() => { if (UI.Radio("UI.Radio", radio == 2)) radio = 2; });

		UI.WindowEnd();
	}

	Pose   sliderWindowPose = new Pose(0.15f, 0, 0);
	float  sliderValf1 = 0;
	float  sliderValf2 = 0;
	float  sliderValf3 = 0;
	double sliderVald  = 0;
	void ShowSliderWindow()
	{
		float sliderTime = (Time.Totalf % 3.0f) / 3.0f;
		UI.WindowBegin("Slides & Separators", ref sliderWindowPose, new Vec2(0.25f, 0.2f));

		UI.LayoutPushCut(UICut.Right, UI.LineHeight*4 + UI.Settings.gutter*3);
		Unique(() => UI.VSlider("UI.VSlider", ref sliderValf2, 0, 1, 0, 0, UIConfirm.Push));
		UI.SameLine();
		Unique(() => UI.VSlider("UI.VSlider", ref sliderValf3, 0, 1, 0, 0, UIConfirm.VariablePinch));
		UI.SameLine();
		UI.VProgressBar(sliderTime);
		UI.SameLine();
		UI.VProgressBar(sliderTime, 0, true);
		UI.LayoutPop();

		Unique(() => UI.HSlider("UI.HSlider", ref sliderValf1, 0, 1, 0, 0, UIConfirm.Push));
		Unique(() => UI.HSlider("UI.HSlider", ref sliderVald,  0, 1, 0, 0, UIConfirm.VariablePinch));
		UI.HSeparator();
		UI.HProgressBar(sliderTime);
		UI.HProgressBar(sliderTime, 0, true);

		UI.WindowEnd();
	}

	Pose   textWindowPose = new Pose(-0.15f, 0, 0);
	string textInput      = "Text here please :)";
	string textPassword   = "Password";
	void ShowTextWindow()
	{
		UI.WindowBegin("Text", ref textWindowPose, V.XY(0.25f,0));

		UI.Label("UI.Label", true);
		UI.SameLine();
		UI.Label("UI.Label", new Vec2(0.14f,0));
		UI.Label("UI.Label", false);

		UI.HSeparator();

		UI.Text("UI.Text", TextAlign.TopLeft);
		UI.Text("UI.Text", TextAlign.TopCenter);
		UI.Text("UI.Text", TextAlign.TopRight);

		UI.HSeparator();

		Vec2 size = new Vec2(0.14f, 0.03f);
		UI.Text("UI.Text with a size overload + Clip",     TextAlign.TopLeft, TextFit.Clip,     size);
		UI.Text("UI.Text with a size overload + Exact",    TextAlign.TopLeft, TextFit.Exact,    size);
		UI.Text("UI.Text with a size overload + Overflow", TextAlign.TopLeft, TextFit.Overflow, size);
		UI.Text("UI.Text with a size overload + Squeeze",  TextAlign.TopLeft, TextFit.Squeeze,  size);
		UI.Text("UI.Text with a size overload + Wrap",     TextAlign.TopLeft, TextFit.Wrap,     size);

		UI.HSeparator();

		UI.Label("UI.Input");
		UI.SameLine();
		UI.Input("Input", ref textInput);
		UI.Label("UI.Input Password");
		UI.SameLine();
		UI.Input("Password", ref textPassword, type: TextContext.Password);

		UI.WindowEnd();
	}

	Pose layoutWindowPose = new Pose(-0.45f, 0, 0);
	void ShowLayouts()
	{
		UI.WindowBegin("Layouts", ref layoutWindowPose, V.XY(0.25f, 0.25f));

		UI.LayoutPushCut(UICut.Left, 0.05f);
			UI.PanelAt(UI.LayoutAt, UI.LayoutRemaining);
			UI.LayoutPushCut(UICut.Top, 0.075f);
				UI.PanelAt(UI.LayoutAt+V.XYZ(0,0,-0.001f), UI.LayoutRemaining);
			UI.LayoutPop();
			UI.LayoutPushCut(UICut.Bottom, 0.075f);
				UI.PanelAt(UI.LayoutAt+V.XYZ(0,0,-0.001f), UI.LayoutRemaining);
			UI.LayoutPop();
		UI.LayoutPop();

		UI.LayoutPushCut(UICut.Right, 0.05f);
		UI.PanelAt(UI.LayoutAt, UI.LayoutRemaining);
		UI.LayoutPop();

		UI.LayoutPushCut(UICut.Bottom, 0.05f);
		UI.PanelAt(UI.LayoutAt, UI.LayoutRemaining);
		UI.LayoutPop();

		UI.LayoutPushCut(UICut.Top, 0.05f);
			UI.PanelAt(UI.LayoutAt, UI.LayoutRemaining);
			UI.LayoutPushCut(UICut.Right, 0.025f);
				UI.PanelAt(UI.LayoutAt+V.XYZ(0,0,-0.001f), UI.LayoutRemaining);
			UI.LayoutPop();
			UI.LayoutPushCut(UICut.Left, 0.025f);
				UI.PanelAt(UI.LayoutAt+V.XYZ(0,0,-0.001f), UI.LayoutRemaining);
			UI.LayoutPop();
		UI.LayoutPop();

		UI.PanelAt(UI.LayoutAt, UI.LayoutRemaining);
		UI.WindowEnd();
	}

	public void Step()
	{
		index = 0;
		Hierarchy.Push(Demo.contentPose);
		ShowButtonWindow();
		ShowSliderWindow();
		ShowTextWindow  ();
		ShowLayouts     ();

		Vec3 off  = new Vec3(0, -0.2f, 0);
		Vec3 at   = Hierarchy.ToWorld(Vec3.Zero + off);
		Vec3 from = Hierarchy.ToWorld(new Vec3(0,0,-0.55f) + off);
		Hierarchy.Pop();

		Tests.Screenshot("Tests/UITearsheet.jpg", 1, 1000, 500, 60, from, at);
		Demo.ShowSummary(title, description, new Bounds(V.XY0(0,-0.23f), V.XYZ(1.3f, .6f, .1f)));
	}
}