using System;
using StereoKit;

namespace SKTemplate_NetCore
{
    class Program
    {
        static void Main(string[] args)
        {
            if (!StereoKitApp.Initialize("StereoKit C#", Runtime.MixedReality, true))
                Environment.Exit(1);

            Model cube = new Model(Mesh.GenerateRoundedCube(Vec3.One, 0.1f), Material.Find("default/material"));

            while (StereoKitApp.Step(() =>
            {
                Renderer.Add(cube, Matrix.Identity, Color.White);
            }));

            StereoKitApp.Shutdown();
        }
    }
}