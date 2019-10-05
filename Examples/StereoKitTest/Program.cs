using System;
using StereoKit;

class Program 
{
    public const string Root = "../Examples/Assets/";

    static Model     floorMesh;
    static Transform floorTr;
    static Solid     floorSolid;

    static IScene activeScene;
    static IScene nextScene;
    public static IScene ActiveScene { get{ return activeScene;} set { nextScene = value; } }
    static void Main(string[] args) 
    {
        StereoKitApp.settings.shaderCache = Program.Root;
        if (!StereoKitApp.Initialize("StereoKit C#", Runtime.Flatscreen, true))
            Environment.Exit(1);
        CommonInit();

        activeScene = new DemoUI();
        activeScene.Initialize();

        while (StereoKitApp.Step(() =>
        {
            if (nextScene != null)
            {
                activeScene.Shutdown();
                nextScene.Initialize();
                activeScene = nextScene;
                nextScene = null;
            }
            CommonUpdate();
            activeScene.Update();
        }));

        activeScene.Shutdown();
        CommonShutdown();

        StereoKitApp.Shutdown();
    }

    static void CommonInit()
    {
        Tex2D cubemap = Tex2D.FromEquirectangular(Program.Root + "Sky/sky.hdr");
        Renderer.SetSkytex(cubemap);

        Material floorMat = new Material(Shader.Find("default/shader_pbr"));
        floorMat["diffuse"  ] = new Tex2D(Program.Root + "test.png");
        floorMat["normal"   ] = new Tex2D(Program.Root + "test_normal.png");
        floorMat["tex_scale"] = 6;

        floorMesh = new Model(Mesh.GenerateCube(Vec3.One), floorMat);
        floorTr   = new Transform(new Vec3(0, -1.5f, 0), new Vec3(5, 1, 5));

        floorSolid = new Solid(floorTr.Position, floorTr.Rotation, SolidType.Immovable);
        floorSolid.AddBox(floorTr.Scale);
    }
    static void CommonUpdate()
    {
        Renderer.Add(floorMesh, floorTr, Color.White);
    }
    static void CommonShutdown()
    {
    }
}