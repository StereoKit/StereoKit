using System;
using StereoKit;

class Program 
{
    static void Main(string[] args) 
    {
        if (!StereoKitApp.Initialize("StereoKit C#", Runtime.MixedReality, true))
            Environment.Exit(1);

        Model     gltf   = new Model("../Examples/Assets/DamagedHelmet.gltf");
        Transform gltfTr = new Transform(Vec3.Zero, Vec3.One*0.25f);
        
        while (StereoKitApp.Step(() => {
            Vec3 lookat = new Vec3((float)Math.Cos(StereoKitApp.Time) * 0.5f, 0, (float)Math.Sin(StereoKitApp.Time) * 0.5f);
            
            Hand hand = Input.Hand(Handed.Right);
            if (hand.IsPinched)
                lookat = hand.root.position; 
            
            gltfTr.LookAt(-lookat);
            Renderer.Add(gltf, gltfTr);
        }));

        StereoKitApp.Shutdown();
    }
}