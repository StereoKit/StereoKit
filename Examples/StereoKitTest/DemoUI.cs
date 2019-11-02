using StereoKit;

class DemoUI : IDemo
{
    float  slider1 = 0.5f;
    float  slider2 = 0.5f;
    Pose   window_pose = new Pose(new Vec3(0.85f, 1, 0.9f), Quat.Lookat(Vec3.Zero, -Vec3.One));

    public void Initialize()
    {
    }

    public void Update()
    {
        UI.WindowBegin("Main", ref window_pose, new Vec2(24, 0) * Units.cm2m);

        UI.Button("Testing!\nok"); UI.SameLine();
        UI.Button("Another");
        UI.HSlider("slider",  ref slider1, 0, 1, 0.2f, 72 * Units.mm2m); UI.SameLine();
        UI.HSlider("slider2", ref slider2, 0, 1, 0,    72 * Units.mm2m);
        if (UI.Button("Press me!"))
            UI.Button("DYANMIC BUTTON!!");

        UI.WindowEnd();
    }

    public void Shutdown()
    {
    }
}