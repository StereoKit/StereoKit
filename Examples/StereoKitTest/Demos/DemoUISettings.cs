using StereoKit;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

class DemoUISettings : ITest
{
	readonly static UISettings defaultSettings = new UISettings {
		backplateBorder = 1f * Units.mm2m,
		backplateDepth  = 0.4f,
		depth           = 10 * Units.mm2m,
		gutter          = 10 * Units.mm2m,
		padding         = 10 * Units.mm2m
	};

	Pose              settingsPose   = new Pose( .25f, 0, 0, Quat.LookDir(-Vec3.Right));
	Pose              colorPose      = new Pose(-.25f, 0, 0, Quat.LookDir( Vec3.Right));
	static UISettings globalSettings = defaultSettings;
	static Vec3       colorScheme    = new Vec3(0.07f, 0.5f, 0.8f);

	public void Update()
	{
		WindowSettings();
	}

	void WindowSettings()
	{
		UI.Settings = defaultSettings;
		UI.WindowBegin("Settings", ref settingsPose, new Vec2(24, 0) * U.cm);

		UI.Label("Spacing");
		UI.Label("Padding"); UI.SameLine();
		UI.HSlider("sl_padding", ref globalSettings.padding, 0, 40*U.mm, 0);
		UI.Label("Gutter"); UI.SameLine();
		UI.HSlider("sl_gutter",  ref globalSettings.gutter,  0, 40*U.mm, 0);

		UI.Label("Depth"); UI.SameLine();
		UI.HSlider("sl_depth",   ref globalSettings.depth,   0, 40*U.mm, 0);

		UI.Label("Backplate");
		UI.Label("Depth"); UI.SameLine();
		UI.HSlider("sl_pl_depth", ref globalSettings.backplateDepth, 0, 0.99f, 0);
		UI.Label("Border"); UI.SameLine();
		UI.HSlider("sl_pl_border", ref globalSettings.backplateBorder, 0, 10 * U.mm, 0);

		UI.WindowEnd();


		UI.WindowBegin("Color", ref colorPose, new Vec2(20, 0) * U.cm);

		float h = colorScheme.x;
		float s = colorScheme.y;
		float v = colorScheme.z;
		UI.HSlider("x", ref h, 0, 1, 0, 18 * U.cm);
		UI.HSlider("y", ref s, 0, 1, 0, 18 * U.cm);
		UI.HSlider("z", ref v, 0, 1, 0, 18 * U.cm);
		colorScheme = new Vec3(h, s, v);
		Color color = Color.HSV(colorScheme.x, colorScheme.y, colorScheme.z);
		Lines.Add(new Vec3(9, -16, 0) * U.cm, new Vec3(-9, -16, 0) * U.cm, color.ToLinear(), .01f);
		UI.ColorScheme = color;

		UI.WindowEnd();
		UI.Settings = globalSettings;
	}

	public void Initialize() { }
	public void Shutdown() { }
}
