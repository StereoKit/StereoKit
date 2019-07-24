using System;
using StereoKit;

class Program {
    static void Main(string[] args) {
        StereoKitApp kit = new StereoKitApp("CSharp OpenXR", Runtime.Flatscreen);

        Model     cube   = new Model("Assets/cube.obj");
        Transform cubeTr = new Transform(Vec3.Zero, Vec3.One * 0.5f);

        Camera    camera   = new Camera(90, 0.1f, 50);
        Transform cameraTr = new Transform(Vec3.One);
        cameraTr.LookAt(Vec3.Zero);
        Renderer.SetCamera(camera, cameraTr);

        while (kit.Step(() => {
            cubeTr.Scale = Vec3.One * 0.5f;
            cubeTr.Pos   = new Vec3((float)Math.Cos(StereoKitApp.Time)*0.5f, 0, (float)Math.Sin(StereoKitApp.Time)*0.5f);
            cubeTr.LookAt(Vec3.Zero);

            Renderer.Add(cube, cubeTr);

            for (int i = 0; i < Input.PointerCount(); i++) {
                Pointer p = Input.Pointer(i);
                if (!p.IsAvailable)
                    continue;

                cubeTr.Pos   = p.ray.pos + p.ray.dir;
                cubeTr.Rot   = Quat.Identity;
                cubeTr.Scale = Vec3.One * 0.1f;
                Renderer.Add(cube, cubeTr);
            }
        }));
    }
}