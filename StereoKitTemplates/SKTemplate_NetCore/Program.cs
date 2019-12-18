using System;
using StereoKit;

namespace SKTemplate_NetCore_Name
{
    class Program
    {
        static void Main(string[] args)
        {
            if (!StereoKitApp.Initialize("SKTemplate_NetCore_Name", Runtime.MixedReality))
                Environment.Exit(1);

            Model cube = Model.FromMesh(
                Mesh.GenerateRoundedCube(Vec3.One, 0.2f), 
                Material.Find(DefaultIds.material));

            while (StereoKitApp.Step(() =>
            {
                cube.Draw(Matrix.TS(Vec3.Zero, 0.1f));
            }));

            StereoKitApp.Shutdown();
        }
    }
}