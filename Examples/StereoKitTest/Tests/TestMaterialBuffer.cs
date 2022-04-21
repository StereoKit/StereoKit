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
			testBuffer = new MaterialBuffer<MaterialBufferStruct>(13);
			testBuffer.Set(data);
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
		Log.Info("NOTE: Expected error log message following.");
		try
		{
			// ids for 0-2 should see MaterialBuffer throw an exceptions
			testBuffer = new MaterialBuffer<MaterialBufferStruct>(2);
		}
		catch { return true; }
		return false;
	}

	bool MaterialBufferIdOverlap()
	{
		Log.Info("NOTE: Expected error log message following.");
		try
		{
			// 13 is already used by MaterialBufferTest, so it should throw
			testBuffer = new MaterialBuffer<MaterialBufferStruct>(13);
		}
		catch { return true; }
		return false;
	}

	public void Initialize()
	{
		Tests.Test(MaterialBufferTest);
		Tests.Test(MaterialBufferId);
		Tests.Test(MaterialBufferIdOverlap);
	}

	public void Shutdown() { }
	public void Update() { }
}