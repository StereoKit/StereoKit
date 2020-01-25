using StereoKit;

class DemoText : IDemo
{
    TextAlign align       = TextAlign.YTop | TextAlign.XLeft;
    Pose      alignWindow = new Pose(0.4f, 0, 0, Quat.LookDir(-1, 0, 1));

    /// :CodeSample: Text.MakeStyle Font.FromFile Text.Add
    /// ### Drawing text with and without a TextStyle
    /// ![Basic text]({{site.url}}/img/screenshots/BasicText.jpg)
    /// We can use a TextStyle object to control how text gets displayed!
    TextStyle style;
    /// :End:
    
    public void Initialize() {
        /// :CodeSample: Text.MakeStyle Font.FromFile Text.Add
        /// In initialization, we can create the style from a font, a size, a 
        /// material, and a base color.
        style = Text.MakeStyle(
            Font.FromFile("C:/Windows/Fonts/Gabriola.ttf"), 
            3 * Units.cm2m,
            Material.Copy(DefaultIds.materialFont), 
            Color.HSV(0.05f, 0.7f, 0.8f));
        /// :End:
    }

    public void Shutdown() { }

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

        Hierarchy.Push(Matrix.T(0, -0.1f, 0));
        if (Demos.TestMode)
            Renderer.Screenshot(Hierarchy.ToWorld(new Vec3(0, 0, 0.09f)), Hierarchy.ToWorld(Vec3.Zero), 600, 300, "../../../docs/img/screenshots/BasicText.jpg");
        /// :CodeSample: Text.MakeStyle Font.FromFile Text.Add
        /// Then it's pretty trivial to just draw some text on the screen! Just call
        /// Text.Add on update. If you don't have a TextStyle available, calling it 
        /// without one will just fall back on the default style.
        // Text with an explicit text style
        Text.Add(
            "Here's\nSome\nMulti-line\nText!!", 
            Matrix.TRS(new Vec3(0.1f, 0, 0), Quat.LookDir(0, 0, 1)), 
            style);
        // Text using the default text style
        Text.Add(
            "Here's\nSome\nMulti-line\nText!!", 
            Matrix.TRS(new Vec3(-0.1f, 0, 0), Quat.LookDir(0, 0, 1)));
        /// :End:
        
        Hierarchy.Push(Matrix.T(0, -0.2f, 0));
        Text.Add(
            "Here's Some Multi-line Text!!",
            Matrix.TRS(new Vec3(0, 0.0f, 0), Quat.LookDir(0, 0, 1)),
            new Vec2(SKMath.Cos(Time.Totalf)*10+11, 20) * Units.cm2m,
            TextFit.Wrap,
            style, TextAlign.Center, align);
        Hierarchy.Pop();

        Hierarchy.Pop();

        Hierarchy.Pop();
    }
}