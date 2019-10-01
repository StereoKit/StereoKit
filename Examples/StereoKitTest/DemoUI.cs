using StereoKit;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

class DemoUI : IScene
{
    Model cube;
    Model gltf;
    Transform solidTr;
    Transform floorTr;
    Solid floorSolid;

    float slider1 = 0.5f;
    float slider2 = 0.5f;
    Pose  window_pose = new Pose(new Vec3(0.85f, 1, 0.9f), Quat.Lookat(Vec3.Zero, -Vec3.One));

    List<Solid> objects = new List<Solid>();

    public void Initialize()
    {
        string root = "../Examples/Assets/";
        Tex2D cubemap = Tex2D.FromEquirectangular(root + "Sky/sky.hdr");
        Renderer.SetSkytex(cubemap);

        Material floorMat = new Material(Shader.Find("default/shader_pbr"));
        floorMat["diffuse"] = new Tex2D(root + "test.png");
        floorMat["normal"] = new Tex2D(root + "test_normal.png");
        floorMat["tex_scale"] = 6;

        cube = new Model(Mesh.GenerateCube("app/mesh_cube", Vec3.One), floorMat);
        gltf = new Model(root + "DamagedHelmet.gltf");
        floorTr = new Transform(new Vec3(0, -1.5f, 0), new Vec3(5, 1, 5));
        solidTr = new Transform(Vec3.Zero, Vec3.One * 0.25f);

        floorSolid = new Solid(floorTr.Position, floorTr.Rotation, SolidType.Immovable);
        floorSolid.AddBox(floorTr.Scale);

        UI.Init();
    }

    public void Update()
    {
        Renderer.Add(cube, floorTr);

        UI.WindowBegin("Main", ref window_pose, new Vec2( 24, 0) * Units.cm2m);

        UI.Button("Testing!\nok");
        UI.Button("Another");
        UI.NextLine();
        UI.HSlider("slider",  ref slider1, 0, 1, 0.2f, 72 * Units.mm2m);
        UI.HSlider("slider2", ref slider2, 0, 1, 0, 72 * Units.mm2m);
        UI.NextLine();
        if (UI.Button("Press me!"))
        {
            UI.NextLine();
            UI.Button("DYANMIC BUTTON!!");
        }

        UI.WindowEnd();
    }

    public void Shutdown()
    {
    }
}