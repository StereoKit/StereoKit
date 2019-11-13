using StereoKit;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

class DemoUISettings : IDemo
{
    Pose       settingsPose = new Pose(new Vec3(.25f, 0, 0), Quat.LookDir(-Vec3.Right));
    UISettings settings;

    public void Update()
    {
        WindowSettings();
    }

    void WindowSettings()
    {
        UI.WindowBegin("Settings", ref settingsPose, new Vec2(24, 0) * Units.cm2m);

        UI.Label("Spacing");
        UI.Label("Padding"); UI.SameLine();
        UI.HSlider("sl_padding", ref settings.padding, 0, 40*Units.mm2m, 0);
        UI.Label("Gutter"); UI.SameLine();
        UI.HSlider("sl_gutter", ref settings.gutter, 0, 40*Units.mm2m, 0);

        UI.Label("Depth"); UI.SameLine();
        UI.HSlider("sl_depth", ref settings.depth, 0, 40 * Units.mm2m, 0);

        UI.Label("Backplate");
        UI.Label("Depth"); UI.SameLine();
        UI.HSlider("sl_pl_depth", ref settings.backplateDepth, 0, 0.99f, 0);
        UI.Label("Border"); UI.SameLine();
        UI.HSlider("sl_pl_border", ref settings.backplateBorder, 0, 10 * Units.mm2m, 0);

        UI.WindowEnd();
        UI.Settings = settings;
    }

    public void Initialize() {
        settings.backplateBorder = 0.5f * Units.mm2m;
        settings.backplateDepth = 0.4f;
        settings.depth = 15 * Units.mm2m;
        settings.gutter = 20 * Units.mm2m;
        settings.padding = 10 * Units.mm2m;
    }
    public void Shutdown() { }
}
