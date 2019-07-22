using System;
using StereoKit;

class Program
{
    static void Main(string[] args)
    {
        StereoKitApp kit = new StereoKitApp("CSharp OpenXR", Runtime.Flatscreen);

        Mesh      cube   = new Mesh  ("Assets/cube.obj");
        Tex2D     tex    = new Tex2D ("Assets/test.png");
        Shader    shader = new Shader("Assets/shader.hlsl");
        Material  mat    = new Material (shader, tex);
        Transform cubeTr = new Transform(Vec3.Zero, new Vec3(0.5f, 0.5f, 0.5f));

        Camera    camera   = new Camera(90, 0.1f, 50);
        Transform cameraTr = new Transform(Vec3.One);
        cameraTr.LookAt(new Vec3(0,0,0));
        Renderer.SetCamera(camera, cameraTr);

        while (kit.Step(() => {
            cubeTr.Pos = new Vec3((float)Math.Cos(StereoKitApp.Time)*0.5f, 0, (float)Math.Sin(StereoKitApp.Time)*0.5f);
            cubeTr.LookAt(Vec3.Zero);

            Renderer.Add(cube, mat, cubeTr);
        }));
    }
}