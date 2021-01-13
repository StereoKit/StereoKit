using StereoKit;

namespace StereoKitApp
{
	public class App : ISKApp
	{
		public SKSettings Settings => new SKSettings { 
			appName           = "StereoKit Template",
			assetsFolder      = "Assets",
			displayFallback   = true,
			displayPreference = DisplayMode.MixedReality
		};

		Pose  cubePose = new Pose(0, 0, -0.5f, Quat.Identity);
		Model cube;

		public void Init()
		{
			// Create assets used by the app
			cube = Model.FromMesh(
				Mesh.GenerateRoundedCube(Vec3.One * 0.1f, 0.02f),
				Default.MaterialUI);
		}

		public void Step()
		{
			UI.Handle("Cube", ref cubePose, cube.Bounds);
			cube.Draw(cubePose.ToMatrix());
		}
	}
}