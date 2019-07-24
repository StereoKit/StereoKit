using System;
using StereoKit;

class Program {
    static void Main(string[] args) {
        StereoKitApp kit = new StereoKitApp("CSharp OpenXR", Runtime.Flatscreen);

        Model     cube   = new Model("Assets/cube.obj");
        Model     gltf   = new Model("../StereoKitCTest/Assets/DamagedHelmet.gltf");
        Transform cubeTr = new Transform(Vec3.Zero, Vec3.One*0.1f);
        Transform gltfTr = new Transform(Vec3.Zero);

        Camera    camera   = new Camera(90, 0.1f, 50);
        Transform cameraTr = new Transform(Vec3.One);
        cameraTr.LookAt(Vec3.Zero);
        Renderer.SetCamera(camera, cameraTr);

        while (kit.Step(() => {
            Vec3 lookat = new Vec3((float)Math.Cos(StereoKitApp.Time) * 0.5f, 0, (float)Math.Sin(StereoKitApp.Time) * 0.5f);
            
            for (int i = 0; i < Input.PointerCount(); i++) {
                Pointer p = Input.Pointer(i);
                if (!p.IsPressed)
                    continue;

                lookat = p.ray.pos + p.ray.dir;

                cubeTr.Pos = lookat;
                Renderer.Add(cube, cubeTr);
            }

            gltfTr.LookAt(Vec3.Zero-lookat);
            Renderer.Add(gltf, gltfTr);
        }));
    }
}