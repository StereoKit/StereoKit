using StereoKit;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

internal class TestTextures : ITest
{
	Tex      failedTex;
	Material failedMat;

	Tex      failedFallbackTex;
	Material failedFallbackMat;

	struct FormatTest
	{
		public TexFormat format;
		public Tex       tex;
		public Material  mat;
	}
	List<FormatTest> testTextures = new();

	public void Initialize()
	{
		Log.Info("Expected texture load failure:");
		failedTex = Tex.FromFile("file_that_doesnt_exist.png");
		failedMat = Material.Default.Copy();
		failedMat[MatParamName.DiffuseTex] = failedTex;

		Log.Info("Expected texture load failure:");
		failedFallbackTex = Tex.FromFile("file_that_doesnt_exist2.png");
		failedFallbackTex.FallbackOverride = Tex.Black;
		failedFallbackMat = Material.Default.Copy();
		failedFallbackMat[MatParamName.DiffuseTex] = failedFallbackTex;

		ulong on64  = 0xFFFF00000000FFFF;
		ulong off64 = 0xFFFF000000000000;

		// Test a NPOT size to ensure mips work properly with that
		int w = 32;
		int h = 32;
		int ow = 41;
		int oh = 32;
		testTextures.Add(MakeTest(TexFormat.Rgba32,       new Color32(255,0,0,255), new Color32(0,0,0,255), w,  h ));
		testTextures.Add(MakeTest(TexFormat.Rgba32Linear, new Color32(255,0,0,255), new Color32(0,0,0,255), ow, oh));
		testTextures.Add(MakeTest(TexFormat.Bgra32,       new Color32(0,0,255,255), new Color32(0,0,0,255), w,  h ));
		testTextures.Add(MakeTest(TexFormat.Bgra32Linear, new Color32(0,0,255,255), new Color32(0,0,0,255), ow, oh));
		testTextures.Add(MakeTest(TexFormat.Rgba64u,      on64, off64, w,  h ));
		testTextures.Add(MakeTest(TexFormat.Rgba64u,      on64, off64, ow, oh));
		testTextures.Add(MakeTest(TexFormat.Rgba128,      new Color(1,0,0,1), new Color(0,0,0,1), w,  h ));
		testTextures.Add(MakeTest(TexFormat.Rgba128,      new Color(1,0,0,1), new Color(0,0,0,1), ow, oh));
		testTextures.Add(MakeTest(TexFormat.R8,           (byte)255, (byte)0, w,  h ));
		testTextures.Add(MakeTest(TexFormat.R8,           (byte)255, (byte)0, ow, oh));
		testTextures.Add(MakeTest(TexFormat.R16,          (ushort)ushort.MaxValue, (ushort)0, w,  h ));
		testTextures.Add(MakeTest(TexFormat.R16,          (ushort)ushort.MaxValue, (ushort)0, ow, oh));
		testTextures.Add(MakeTest(TexFormat.R32,          1.0f, 0, w,  h ));
		testTextures.Add(MakeTest(TexFormat.R32,          1.0f, 0, ow, oh));

		Tests.Screenshot("TexFormats.jpg", 0, 400, 400, 50, V.XYZ(0,-0.15f, 0), V.XYZ(0,-0.15f,-0.5f) );
	}

	public void Step()
	{
		Mesh.Cube.Draw(failedMat,         Matrix.TS(-0.1f, 0.2f, -0.5f, 0.1f));
		Mesh.Cube.Draw(failedFallbackMat, Matrix.TS( 0.1f, 0.2f, -0.5f, 0.1f));

		float s = 0.1f;
		int w = 4;
		for (int i = 0; i < testTextures.Count; i++)
		{
			int x = i % w;
			int y = i / w;
			Vec3 at = new Vec3(-s*((w-1)/2.0f) + s*x, -y*s, -0.5f);
			Quat dir = Quat.LookDir(0,0,1);
			Mesh.Quad.Draw(testTextures[i].mat, Matrix.TRS(at, dir, s* 0.9f));
			Text.Add(testTextures[i].format.ToString(), Matrix.TRS(at + V.XYZ(0,0,0.02f), dir, 0.5f));
		}
	}

	public void Shutdown()
	{
	}

	static FormatTest MakeTest<T>(TexFormat format, T a, T b, int w, int h)
	{
		FormatTest result = new FormatTest
		{
			format = format,
			tex = new Tex(TexType.Image, format),
			mat = Material.Default.Copy()
		};

		int    size  = Marshal.SizeOf<T>();
		IntPtr ptr   = Marshal.AllocHGlobal(size * w * h);
		long   start = ptr.ToInt64();
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				IntPtr dest = new IntPtr(start + (x + y * w) * size);
				Marshal.StructureToPtr((x % 4 == 0 || y % 4 == 0) ? a : b, dest, false);
			}
		}
		result.tex.SetColors(w, h, ptr);
		Marshal.FreeHGlobal(ptr);

		result.mat[MatParamName.DiffuseTex] = result.tex;

		return result;
	}
}