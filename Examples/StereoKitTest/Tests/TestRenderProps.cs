using StereoKit;

class TestRenderProps : ITest
{
	static bool TestClip()
	{
		bool result = true;
		Renderer.GetClip(out float oldNear, out float oldFar);
		Log.Info($"Default Clip: Near {oldNear}, Far {oldFar}");

		Renderer.SetClip(0.1f, 1000.0f);
		Renderer.GetClip(out float newNear, out float newFar);
		if (newNear != 0.1f || newFar != 1000.0f)
			result = false;

		Renderer.SetClip(oldNear, oldFar);
		return result;
	}

	static bool TestFOV()
	{
		bool result = true;
		float oldFOV = Renderer.FOV;
		Log.Info($"Default FOV: {oldFOV}");

		Renderer.FOV = 45.0f;
		if (Renderer.FOV != 45.0f)
			result = false;

		Renderer.FOV = oldFOV;
		return result;
	}

	static bool TestOrthoSize()
	{
		bool result = true;
		float oldOrthoSize = Renderer.OrthoSize;
		Log.Info($"Default OrthoSize: {oldOrthoSize}");

		Renderer.OrthoSize = 0.5f;
		if (Renderer.OrthoSize != 0.5f)
			result = false;

		Renderer.OrthoSize = oldOrthoSize;
		return result;
	}

	static bool TestGPUMemory()
	{
		GPUMemory before = Renderer.GPUMemory;
		Log.Info($"GPU memory: {before.usedBytes / 1048576.0:0.0} MB used, {before.reservedBytes / 1048576.0:0.0} MB reserved");
		if (before.usedBytes == 0 || before.reservedBytes < before.usedBytes || before.peakBytes < before.usedBytes)
			return false;

		const int size = 256;
		Tex tex = Tex.FromColors(new Color32[size * size], size, size);
		Assets.BlockForPriority(int.MaxValue);
		GPUMemory after = Renderer.GPUMemory;
		System.GC.KeepAlive(tex);
		return after.textureBytes >= before.textureBytes + size * size * 4;
	}

	public void Initialize()
	{
		Tests.Test(TestClip);
		Tests.Test(TestFOV);
		Tests.Test(TestOrthoSize);
		Tests.Test(TestGPUMemory);
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
	}
}
