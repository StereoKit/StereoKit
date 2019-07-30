using System;
using StereoKit;

class Program 
    {
    static void Main(string[] args) 
    {
        StereoKitApp kit = new StereoKitApp("CSharp OpenXR", Runtime.MixedReality);
        Model     gltf   = new Model("Assets/DamagedHelmet.gltf");
        Model     hand   = new Model("Assets/cube.obj");
        Transform gltfTr = new Transform(Vec3.Zero);
        Transform handTr = new Transform(Vec3.Zero);
        
        while (kit.Step(() => {
            Vec3 lookat = new Vec3((float)Math.Cos(StereoKitApp.Time) * 0.5f, 0, (float)Math.Sin(StereoKitApp.Time) * 0.5f);
            
            for (int i = 0; i < Input.PointerCount(PointerSource.CanPress); i++) {
                Pointer pointer = Input.Pointer(i, PointerSource.CanPress);
                handTr.Position = pointer.ray.position;
                handTr.Rotation = pointer.orientation;
                Renderer.Add(hand, handTr);

                if (pointer.IsPressed)
                    lookat = pointer.ray.position + pointer.ray.direction;
            }
            
            gltfTr.LookAt(lookat);
            Renderer.Add(gltf, gltfTr);
        }));
    }
}