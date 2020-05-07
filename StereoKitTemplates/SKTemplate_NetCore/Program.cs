using System;
using StereoKit;

namespace SKTemplate_NetCore_Name
{
	class Program
	{
		static void Main(string[] args)
		{
			// Initialize StereoKit
			StereoKitApp.settings.assetsFolder = "Assets";
			if (!StereoKitApp.Initialize("SKTemplate_NetCore_Name", Runtime.MixedReality))
				Environment.Exit(1);


			// Create assets used by the app
			Pose cubePose = new Pose(0,0,-0.5f, Quat.Identity);
			Model cube = Model.FromMesh(
				Mesh.GenerateRoundedCube(Vec3.One*0.1f, 0.02f), 
				Default.MaterialUI);

			Matrix floorTransform = Matrix.TS(new Vec3(0, -1.5f, 0), new Vec3(30, 0.1f, 30));
			Material floorMaterial = new Material(Shader.FromFile("floor.hlsl"));
			floorMaterial.Transparency = Transparency.Blend;


			// Core application loop
			while (StereoKitApp.Step(() =>
			{
				if (StereoKitApp.System.displayType == Display.Opaque)
					Default.MeshCube.Draw(floorMaterial, floorTransform);

				UI.Handle("Cube", ref cubePose, cube.Bounds);
				cube.Draw(cubePose.ToMatrix());
			}));
			StereoKitApp.Shutdown();
		}
	}
}