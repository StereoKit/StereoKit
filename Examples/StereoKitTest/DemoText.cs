using StereoKit;

class DemoText : IDemo
{
    public void Initialize() { }

    public void Shutdown() { }

    TextAlign align = TextAlign.YTop | TextAlign.XLeft;
    Pose alignWindow = new Pose(0.25f, 0, 0, Quat.LookDir(-1,0,1));

    public void Update()
    {
        Hierarchy.Push(Matrix.T(0, 0, -0.3f));

        UI.WindowBegin("Alignment", ref alignWindow, new Vec2(20,0) * Units.cm2m);
        if (UI.Button("Left"  )) align = TextAlign.YCenter | TextAlign.XLeft;   UI.SameLine();
        if (UI.Button("Center")) align = TextAlign.YCenter | TextAlign.XCenter; UI.SameLine();
        if (UI.Button("Right" )) align = TextAlign.YCenter | TextAlign.XRight;
        UI.WindowEnd();

        Hierarchy.Push(Matrix.T(0.1f,0,0));
        Text.Add("X Center", Matrix.TRS(new Vec3(0,.1f, 0), Quat.LookDir(0,0,1)), TextAlign.XCenter|TextAlign.YCenter, align);
        Text.Add("X Left",   Matrix.TRS(new Vec3(0,.15f,0), Quat.LookDir(0,0,1)), TextAlign.XLeft  |TextAlign.YCenter, align);
        Text.Add("X Right",  Matrix.TRS(new Vec3(0,.2f, 0), Quat.LookDir(0,0,1)), TextAlign.XRight |TextAlign.YCenter, align);
        Lines.Add(new Vec3(0,.05f,0), new Vec3(0,.25f,0), Color32.White, 0.001f);
        Hierarchy.Pop();

        Hierarchy.Push(Matrix.T(-0.1f,0,0));
        Text.Add("Y Center", Matrix.TRS(new Vec3(0,.1f, 0), Quat.LookDir(0,0,1)), TextAlign.YCenter|TextAlign.XCenter, align);
        Lines.Add(new Vec3(-0.05f, .1f, 0), new Vec3(.05f, .1f, 0), Color32.White, 0.001f);

        Text.Add("Y Top",    Matrix.TRS(new Vec3(0,.15f,0), Quat.LookDir(0,0,1)), TextAlign.YTop|TextAlign.XCenter, align);
        Lines.Add(new Vec3(-0.05f, .15f, 0), new Vec3(.05f, .15f, 0), Color32.White, 0.001f);

        Text.Add("Y Bottom", Matrix.TRS(new Vec3(0,.2f, 0), Quat.LookDir(0,0,1)), TextAlign.YBottom|TextAlign.XCenter, align);
        Lines.Add(new Vec3(-0.05f,.2f, 0), new Vec3(.05f,.2f, 0), Color32.White, 0.001f);
        Hierarchy.Pop();
        
        Text.Add("Here's\nSome\nMulti-line\nText!!", Matrix.TRS(Vec3.Zero, Quat.LookDir(0, 0, 1)), TextAlign.Center, align);

        Hierarchy.Pop();
    }
}