using System;
using StereoKit;

class Program 
{
#if WINDOWS_UWP
    const string Root = "Assets";
#else
    const string Root = "../../../Examples/Assets";
#endif

    static Model  floorMesh;
    static Matrix floorTr;
    static Solid  floorSolid;
    static Pose   demoSelectPose = new Pose();

    static void Main(string[] args) 
    {
        Demos.TestMode = args.Length > 0 && args[0].ToLower() == "-test";
        Time .Scale    = Demos.TestMode ? 0 : 1;

        StereoKitApp.settings.assetsFolder = Program.Root;
        if (!StereoKitApp.Initialize("StereoKit C#", Demos.TestMode ? Runtime.Flatscreen : Runtime.MixedReality,  true))
            Environment.Exit(1);
        CommonInit();

        Demos.FindDemos();
        Demos.SetActive(args.Length > 0 ? args[0] : "Geo");
        Demos.Initialize();
        
        while (StereoKitApp.Step(() =>
        {
            Demos.Update();
            CommonUpdate();
        }));

        Demos.Shutdown();
        CommonShutdown();

        StereoKitApp.Shutdown();
    }

    static void CommonInit()
    {
        Material floorMat = Material.Copy(DefaultIds.material);
        floorMat["diffuse"  ] = Tex.FromFile("Floor.png");
        floorMat["tex_scale"] = 16;

        floorMesh = Model.FromMesh(Mesh.GenerateCube(Vec3.One), floorMat);
        floorTr   = Matrix.TRS(new Vec3(0, -1.5f, 0), Quat.Identity, new Vec3(20, 1, 20));

        floorSolid = new Solid(new Vec3(0, -1.5f, 0), Quat.Identity, SolidType.Immovable);
        floorSolid.AddBox(new Vec3(20, 1, 20));

        demoSelectPose.position    = new Vec3(0, 0, -0.25f);
        demoSelectPose.orientation = Quat.LookDir(-Vec3.Forward);
    }
    static void CommonUpdate()
    {
        // If we can't see the world, we'll draw a floor!
        if (StereoKitApp.System.displayType == Display.Opaque)
            Renderer.Add(floorMesh, floorTr, Color.White);

        // Skip selection window if we're in test mode
        if (Demos.TestMode)
            return;

        // Make a window for demo selection
        UI.WindowBegin("Demos", ref demoSelectPose, new Vec2(50 * Units.cm2m, 0));
        for (int i = 0; i < Demos.Count; i++)
        {
            // Chop off the "Demo" part of any demo name that has it
            string name = Demos.GetName(i);
            if (name.StartsWith("Demo"))
                name = name.Substring("Demo".Length);

            if (UI.Button(name))
                Demos.SetActive(i);
            UI.SameLine();
        }
        UI.WindowEnd();

        RulerWindow();
    }
    static void CommonShutdown()
    {
    }

    static Pose demoRuler = new Pose(0, 0, .5f, Quat.Identity);
    static void RulerWindow()
    {
        UI.AffordanceBegin("Ruler", ref demoRuler, Vec3.Zero, new Vec3(30,4,1)*Units.cm2m, true);
        Color32 color = Color.HSV(.6f, 0.5f, 1);
        Text.Add("Centimeters", Matrix.TRS(new Vec3(14.5f, -1.5f, -.6f) * Units.cm2m, Quat.Identity, .3f), TextAlign.XLeft | TextAlign.YBottom);
        for (int d = 0; d <= 60; d+=1)
        {
            float x = d/2.0f;
            float size = d%2==0?1f:0.15f;
            Lines.Add(new Vec3(15-x,2,-.6f)* Units.cm2m, new Vec3(15-x,2-size, -.6f) *Units.cm2m, color, Units.mm2m*0.5f);
            if (d%2==0 && d/2 != 30)
                Text.Add((d/2).ToString(), Matrix.TRS(new Vec3(15-x-0.1f,2-size, -.6f) * Units.cm2m, Quat.Identity, .2f), TextAlign.XLeft|TextAlign.YBottom);
        }
        UI.AffordanceEnd();
    }
}