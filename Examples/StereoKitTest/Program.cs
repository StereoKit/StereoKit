using System;
using StereoKit;

class Program 
{
#if WINDOWS_UWP
    const string Root = "Assets/";
#else
    const string Root = "../../../Examples/Assets/";
#endif

    static Model  floorMesh;
    static Matrix floorTr;
    static Solid  floorSolid;
    static Pose   demoSelectPose = new Pose();
    static bool   demoTestMode = false;
    static int    demoTestIndex = 0;

    static void Main(string[] args) 
    {
        StereoKitApp.settings.assetsFolder = Program.Root;
        if (!StereoKitApp.Initialize("StereoKit C#", Runtime.Flatscreen, true))
            Environment.Exit(1);
        CommonInit();

        Demos.FindDemos();
        Demos.TestMode = args.Length > 0 && args[0].ToLower() == "-test";
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
        Material floorMat = Material.Copy(DefaultIds.material);// new Material(Shader.Find("default/shader_pbr"));
        floorMat["diffuse"  ] = new Tex("test.png");
        floorMat["normal"   ] = new Tex("test_normal.png");
        floorMat["tex_scale"] = 6;

        floorMesh = new Model(Mesh.GenerateCube(Vec3.One), floorMat);
        floorTr   = Matrix.TRS(new Vec3(0, -1.5f, 0), Quat.Identity, new Vec3(5, 1, 5));

        floorSolid = new Solid(new Vec3(0, -1.5f, 0), Quat.Identity, SolidType.Immovable);
        floorSolid.AddBox(new Vec3(5, 1, 5));

        demoSelectPose.position    = new Vec3(0, 0, -0.25f);
        demoSelectPose.orientation = Quat.LookDir(-Vec3.Forward);
    }
    static void CommonUpdate()
    {
        Renderer.Add(floorMesh, floorTr, Color.White);

        // Make a window for demo selection
        UI.WindowBegin("Demos", ref demoSelectPose, new Vec2(50 * Units.cm2m, 0));
        for (int i = 0; i < Demos.Count; i++)
        {
            // Chop off the "Demo" part of any demo name that has it
            string name = Demos.GetName(i);
            if (name.StartsWith("Demo"))
                name = name.Substring("Demo".Length);

            if (UI.ButtonRound(name, 4*Units.cm2m))
                Demos.SetActive(i);
            UI.SameLine();
        }
        UI.WindowEnd();
    }
    static void CommonShutdown()
    {
    }
}