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
			Renderer.SetGlobalBuffer("material_buffer_test", testBuffer);
		} 
		catch (Exception e)
		{
			Log.Err(e.ToString());
			return false;
		}
		return true;
	}

	public void Initialize()
	{
		Tests.Test(MaterialBufferTest);
	}

	public void Shutdown() {
		Renderer.SetGlobalBuffer("material_buffer_test", null);
	}
	public void Step() { }
}