using StereoKit;
using System.IO;

class TestAssetsFromMemory : ITest
{
	bool LoadTexture()
	{
		byte[] file = Platform.ReadFileBytes("floor.png");
		Tex    tex  = Tex.FromMemory(file);
		return tex != null;
	}
	bool LoadModel()
	{
		byte[] file  = Platform.ReadFileBytes("Clipboard.glb");
		Model  model = Model.FromMemory("Clipboard.glb", file);
		return model != null;
	}

	public void Initialize()
	{
		Tests.Test(LoadTexture);
		Tests.Test(LoadModel);
	}

	public void Shutdown(){}
	public void Step(){}
}