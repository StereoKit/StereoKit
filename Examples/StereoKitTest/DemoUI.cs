using StereoKit;

class DemoUI : IDemo
{
    float  slider1 = 0.5f;
    float  slider2 = 0.5f;
    Pose   windowPose   = new Pose(new Vec3(0, 0, -.15f), Quat.LookAt(new Vec3(0,0,-.25f), Vec3.Zero));
    Pose   settingsPose = new Pose(new Vec3(.25f, 0, 0), Quat.LookAt(new Vec3(.25f, 0, 0), Vec3.Zero));

    public void Initialize() { }
    public void Shutdown  () { }

    public void Update()
    {
        WindowTest();
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
        UI.HSlider("sl_pl_depth", ref UI.Settings.backplateDepth, 0, UI.Settings.depth*0.99f, 0);
        UI.Label("Border"); UI.SameLine();
        UI.HSlider("sl_pl_border", ref UI.Settings.backplateBorder, 0, 10 * Units.mm2m, 0);

        UI.WindowEnd();
    }

    void WindowTest()
    {
        UI.WindowBegin("Main", ref windowPose, new Vec2(24, 0) * Units.cm2m);

        UI.Button("Testing!\nok"); UI.SameLine();
        UI.Button("Another");
        UI.HSlider("slider", ref slider1, 0, 1, 0.2f, 72 * Units.mm2m); UI.SameLine();
        UI.HSlider("slider2", ref slider2, 0, 1, 0, 72 * Units.mm2m);
        UI.Button("Press me!");
        UI.ButtonRound("ROUND");

        UI.WindowEnd();
    }
}