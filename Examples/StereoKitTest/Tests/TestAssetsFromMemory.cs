using StereoKit;
using System.IO;

class TestAssetsFromMemory : ITest
{
	bool LoadTexture()
	{
		byte[] file = File.ReadAllBytes(SK.Settings.assetsFolder + "/floor.png");
		Tex tex = Tex.FromMemory(file);
		return tex != null;
	}
	bool LoadModel()
	{
		byte[] file = File.ReadAllBytes(SK.Settings.assetsFolder + "/clipboard.glb");
		Model model = Model.FromMemory(SK.Settings.assetsFolder + "/clipboard.glb", file);
		return model != null;
	}

	public void Initialize()
	{
		Tests.Test(LoadTexture);
		Tests.Test(LoadModel);
	}

	public void Shutdown(){}
	public void Update(){}
}