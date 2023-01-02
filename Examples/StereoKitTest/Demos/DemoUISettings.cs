﻿using StereoKit;

class DemoUISettings : ITest
{
	string title       = "UI Settings";
	string description = "";

	Pose              settingsPose   = Demo.contentPose * new Pose(  0f, 0, 0);
	Pose              colorPose      = Demo.contentPose * new Pose(-.3f, 0, 0);
	UISettings        initialSettings;
	static UISettings activeSettings;
	static Vec3       colorScheme    = new Vec3(0.07f, 0.5f, 0.8f);

	public void Update()
	{
		WindowSettings();
	}

	void WindowSettings()
	{
		UI.Settings = initialSettings;
		UI.WindowBegin("Settings", ref settingsPose, new Vec2(24, 0) * U.cm);

		Vec2 size = new Vec2(0.06f, 0);
		UI.Label("Spacing");

		UI.PanelBegin();
		UI.Label("Padding", size); UI.SameLine();
		UI.HSlider("sl_padding", ref activeSettings.padding, 0, 40*U.mm, 0);
		UI.Label("Gutter", size); UI.SameLine();
		UI.HSlider("sl_gutter",  ref activeSettings.gutter,  0, 40*U.mm, 0);
		UI.Label("Depth", size); UI.SameLine();
		UI.HSlider("sl_depth",   ref activeSettings.depth,   0, 40*U.mm, 0);
		UI.Label("Rounding", size); UI.SameLine();
		UI.HSlider("sl_rounding", ref activeSettings.rounding, 0, 40 * U.mm, 0);
		UI.PanelEnd();

		UI.HSeparator();

		UI.Label("Backplate");

		UI.PanelBegin();
		UI.Label("Depth", size); UI.SameLine();
		UI.HSlider("sl_pl_depth", ref activeSettings.backplateDepth, 0, 0.99f, 0);
		UI.Label("Border", size); UI.SameLine();
		UI.HSlider("sl_pl_border", ref activeSettings.backplateBorder, 0, 10 * U.mm, 0);
		UI.PanelEnd();

		UI.WindowEnd();
		UI.Settings = activeSettings;


		UI.WindowBegin("Color", ref colorPose, new Vec2(20, 0) * U.cm);

		UI.Label("Hue / Saturation / Value");
		float h = colorScheme.x;
		float s = colorScheme.y;
		float v = colorScheme.z;
		UI.PanelBegin();
		UI.HSlider("x", ref h, 0, 1, 0, 18 * U.cm);
		UI.HSlider("y", ref s, 0, 1, 0, 18 * U.cm);
		UI.HSlider("z", ref v, 0, 1, 0, 18 * U.cm);
		UI.PanelEnd();
		colorScheme = new Vec3(h, s, v);
		Color  color  = Color.HSV(colorScheme.x, colorScheme.y, colorScheme.z);
		Bounds bounds = UI.LayoutReserve(Vec2.Zero, false, 0.02f);
		Lines.Add(bounds.center+V.XYZ(bounds.dimensions.x/2,0,0), bounds.center - V.XYZ(bounds.dimensions.x / 2, 0, 0), color.ToLinear(), .02f);
		//Lines.Add(new Vec3(9, -16, 0) * U.cm, new Vec3(-9, -16, 0) * U.cm, color.ToLinear(), .01f);
		UI.ColorScheme = color;

		UI.WindowEnd();

		Demo.ShowSummary(title, description);
	}

	public void Initialize() { activeSettings = UI.Settings; initialSettings = UI.Settings; }
	public void Shutdown() { }
}
