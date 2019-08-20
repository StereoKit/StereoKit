using System;
using System.Collections.Generic;
using StereoKit;

class Program 
{
    static void Main(string[] args) 
    {
        if (!StereoKitApp.Initialize("StereoKit C#", Runtime.Flatscreen, true))
            Environment.Exit(1);

        string root = "../Examples/Assets/";
        Tex2D cubemap = new Tex2D(new string[6] {
            root+"Sky/Right.jpg",  root+"Sky/Left.jpg", root+"Sky/Top.jpg",  
            root+"Sky/Bottom.jpg", root+"Sky/Back.jpg", root+"Sky/Front.jpg" });
        Renderer.SetSkytex(cubemap);

        Material floorMat = new Material("app/mat_floor", Shader.Find("default/shader_pbr"));
        floorMat.SetTexture("diffuse", new Tex2D(root+"test.png" ));
        floorMat.SetTexture("normal",  new Tex2D(root+"test_normal.png"));
        floorMat.SetFloat  ("tex_scale", 6);

        Model     cube   = new Model("app/cube", Mesh.GenerateCube("app/mesh_cube", Vec3.One), floorMat);
        Model     gltf   = new Model(root+"DamagedHelmet.gltf");
        Transform floorTr= new Transform(new Vec3(0,-1.5f, 0), new Vec3(5,1,5));
        Transform solidTr= new Transform(Vec3.Zero, Vec3.One * 0.25f);

        Solid floorSolid = new Solid(floorTr.Position, floorTr.Rotation, SolidType.Immovable);
        floorSolid.AddBox(floorTr.Scale);
        
        List<Solid> objects = new List<Solid>();
        while (StereoKitApp.Step(() => {
            if (Input.Hand(Handed.Right).IsJustPinched) {
                objects.Add(new Solid(new Vec3(0,3,0), Quat.Identity));
                objects[objects.Count-1].AddSphere(.45f, 40);
                objects[objects.Count-1].AddBox   (Vec3.One*.35f, 40);
            }

            for (int i = 0; i < objects.Count; i++) {
                objects[i].GetTransform(ref solidTr);
                Renderer.Add(gltf, solidTr);
            }
            Renderer.Add(cube, floorTr);
        }));

        StereoKitApp.Shutdown();
    }
}