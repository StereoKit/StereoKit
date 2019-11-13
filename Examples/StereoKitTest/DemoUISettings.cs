using StereoKit;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

class DemoUISettings : IDemo
{
    Pose settingsPose = new Pose(new Vec3(.25f, 0, 0), Quat.LookDir(-Vec3.Right));

    public void Update()
    {
        WindowSettings();
    }

    void WindowSettings()
    {
        UI.WindowBegin("Settings", ref settingsPose, new Vec2(24, 0) * Units.cm2m);

        UI.Label("Spacing");
        UI.Label("Padding"); UI.SameLine();
        UI.HSlider("sl_padding", ref UI.Settings.padding, 0, 40*Units.mm2m, 0);
        UI.Label("Gutter"); UI.SameLine();
        UI.HSlider("sl_gutter", ref UI.Settings.gutter, 0, 40*Units.mm2m, 0);

        UI.Label("Depth"); UI.SameLine();
        UI.HSlider("sl_depth", ref UI.Settings.depth, 0, 40 * Units.mm2m, 0);

        UI.Label("Backplate");
        UI.Label("Depth"); UI.SameLine();
        UI.HSlider("sl_pl_depth", ref UI.Settings.backplateDepth, 0, 0.99f, 0);
        UI.Label("Border"); UI.SameLine();
        UI.HSlider("sl_pl_border", ref UI.Settings.backplateBorder, 0, 10 * Units.mm2m, 0);

        UI.WindowEnd();
    }

    public void Initialize() { }
    public void Shutdown() { }
}
