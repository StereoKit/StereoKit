using System;
using StereoKit;

class Program 
{
    static void Main(string[] args) 
    {
        StereoKitApp kit = new StereoKitApp("StereoKit C#", Runtime.Flatscreen);

        //Model gltf = new Model("Assets/DamagedHelmet.gltf");
        Model gltf = new Model("app/model_cube",
            Mesh.GenerateCube("app/mesh_cube", Vec3.One),
            Material.Find("default/material"));
        Transform gltfTr = new Transform(Vec3.Zero, Vec3.One*0.1f);
        
        while (kit.Step(() => {
            Vec3 lookat = new Vec3((float)Math.Cos(StereoKitApp.Time) * 0.5f, 0, (float)Math.Sin(StereoKitApp.Time) * 0.5f);
            
            Hand hand = Input.Hand(Handed.Right);
            if (hand.IsPinched)
                lookat = hand.root.position; 
            
            gltfTr.LookAt(-lookat);
            Renderer.Add(gltf, gltfTr);
        }));
    }
}