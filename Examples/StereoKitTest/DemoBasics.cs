using StereoKit;
using System.Collections.Generic;
using System.Linq;

class DemoBasics : IScene
{
    Model cube;
    Model gltf;
    Transform solidTr;
    Transform floorTr;
    Solid floorSolid;

    List<Solid> objects = new List<Solid>();

    public void Initialize()
    {
        string root = "../Examples/Assets/";
        Tex2D cubemap = Tex2D.FromEquirectangular(root + "Sky/sky.hdr");
        Renderer.SetSkytex(cubemap);

        Material floorMat = new Material(Shader.Find("default/shader_pbr"));
        floorMat["diffuse"  ] = new Tex2D(root + "test.png");
        floorMat["normal"   ] = new Tex2D(root + "test_normal.png");
        floorMat["tex_scale"] = 6;

        cube = new Model(Mesh.GenerateCube("app/mesh_cube", Vec3.One), floorMat);
        gltf = new Model(root + "DamagedHelmet.gltf");
        floorTr = new Transform(new Vec3(0, -1.5f, 0), new Vec3(5, 1, 5));
        solidTr = new Transform(Vec3.Zero, Vec3.One * 0.25f);

        floorSolid = new Solid(floorTr.Position, floorTr.Rotation, SolidType.Immovable);
        floorSolid.AddBox(floorTr.Scale);
    }

    public void Update()
    {
        if (Input.Hand(Handed.Right).IsJustPinched)
        {
            objects.Add(new Solid(new Vec3(0, 3, 0), Quat.Identity));
            objects.Last().AddSphere(.45f, 40);
            objects.Last().AddBox(Vec3.One * .35f, 40);
        }

        for (int i = 0; i < objects.Count; i++)
        {
            objects[i].GetTransform(ref solidTr);
            Renderer.Add(gltf, solidTr);
        }
        Renderer.Add(cube, floorTr);
    }

    public void Shutdown()
    {
    }
}