using System;
using StereoKit;

class Program 
{
#if WINDOWS_UWP
    public const string Root = "Assets/";
#else
    public const string Root = "../../../Examples/Assets/";
#endif

    static Model     floorMesh;
    static Transform floorTr;
    static Solid     floorSolid;
    static Pose      demoSelectPose = new Pose();

    static void Main(string[] args) 
    {
        StereoKitApp.settings.shaderCache = Program.Root;
        if (!StereoKitApp.Initialize("StereoKit C#", Runtime.MixedReality, true))
            Environment.Exit(1);
        CommonInit();

        Demos.FindDemos();
        Demos.SetActive(args.Length > 0 ? args[0] : "Select");
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
        Material floorMat = new Material(Shader.Find("default/shader_pbr"));
        floorMat["diffuse"  ] = new Tex2D(Program.Root + "test.png");
        floorMat["normal"   ] = new Tex2D(Program.Root + "test_normal.png");
        floorMat["tex_scale"] = 6;

        floorMesh = new Model(Mesh.GenerateCube(Vec3.One), floorMat);
        floorTr   = new Transform(new Vec3(0, -1.5f, 0), new Vec3(5, 1, 5));

        floorSolid = new Solid(floorTr.Position, floorTr.Rotation, SolidType.Immovable);
        floorSolid.AddBox(floorTr.Scale);

        demoSelectPose.position    = new Vec3(0, 0, -0.25f);
        demoSelectPose.orientation = Quat.Lookat(demoSelectPose.position, Vec3.Zero);
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

            if (UI.Button(name))
                Demos.SetActive(i);
            UI.SameLine();
        }
        UI.WindowEnd();
    }
    static void CommonShutdown()
    {
    }
}