using StereoKit;
using System;
using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential)]
struct MaterialBufferStruct
{
	public Vec3  point;
	public float scale;
}

class TestMaterialBuffer : ITest
{
	MaterialBuffer<MaterialBufferStruct> testBuffer;

	bool MaterialBufferTest()
	{
		MaterialBufferStruct data = new MaterialBufferStruct {
			point = Vec3.One,
			scale = 2
		};

		try
		{ 
			testBuffer = new MaterialBuffer<MaterialBufferStruct>();
			testBuffer.Set(data);
			Renderer.SetGlobalBuffer(13, testBuffer);
		} 
		catch (Exception e)
		{
			Log.Err(e.ToString());
			return false;
		}
		return true;
	}

	bool MaterialBufferId()
	{
		Log.Warn("Expected error:");
		try
		{
			// ids for 0-2 should see an error message in the log
			testBuffer = new MaterialBuffer<MaterialBufferStruct>();
			Renderer.SetGlobalBuffer(2, testBuffer);
		}
		catch { return false; }
		return true;
	}

	public void Initialize()
	{
		Tests.Test(MaterialBufferTest);
		Tests.Test(MaterialBufferId);
	}

	public void Shutdown() {
		Renderer.SetGlobalBuffer(13, null);
	}
	public void Step() { }
}