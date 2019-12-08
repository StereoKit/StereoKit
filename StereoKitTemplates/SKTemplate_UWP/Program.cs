using System;
using StereoKit;

namespace SKTemplate_UWP
{
    class Program
    {
        static void Main(string[] args)
        {
            if (!StereoKitApp.Initialize("StereoKit C#", Runtime.MixedReality))
                Environment.Exit(1);

            Model cube = Model.FromMesh(
                Mesh    .GenerateRoundedCube(Vec3.One*0.1f, 0.1f), 
                Material.Find(DefaultIds.material));

            while (StereoKitApp.Step(() =>
            {
                cube.Draw(Matrix.Identity, Color.White);
            }));

            StereoKitApp.Shutdown();
        }
    }
}