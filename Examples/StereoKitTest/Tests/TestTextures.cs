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

		// 0x3C00 is a half float with value 1.0f
		ulong  on64  = 0x3C00000000003C00;
		ulong  off64 = 0x3C00000000000000;
		ushort on16  = 0x3C00;
		ushort off16 = 0x0000;

		// Test a NPOT size to ensure mips work properly with that
		int w = 32;
		int h = 32;
		int ow = 41;
		int oh = 33;
		testTextures.Add(MakeTest(TexFormat.Rgba32,       new Color32(255,0,0,255), new Color32(0,0,0,255), w,  h ));
		testTextures.Add(MakeTest(TexFormat.Rgba32Linear, new Color32(255,0,0,255), new Color32(0,0,0,255), ow, oh));
		testTextures.Add(MakeTest(TexFormat.Bgra32,       new Color32(0,0,255,255), new Color32(0,0,0,255), w,  h ));
		testTextures.Add(MakeTest(TexFormat.Bgra32Linear, new Color32(0,0,255,255), new Color32(0,0,0,255), ow, oh));
		testTextures.Add(MakeTest(TexFormat.Rgba64f,      on64, off64, w,  h ));
		testTextures.Add(MakeTest(TexFormat.Rgba64f,      on64, off64, ow, oh));
		testTextures.Add(MakeTest(TexFormat.Rgba128,      new Color(1,0,0,1), new Color(0,0,0,1), w,  h ));
		testTextures.Add(MakeTest(TexFormat.Rgba128,      new Color(1,0,0,1), new Color(0,0,0,1), ow, oh));
		testTextures.Add(MakeTest(TexFormat.R8,           (byte)255, (byte)0, w,  h ));
		testTextures.Add(MakeTest(TexFormat.R8,           (byte)255, (byte)0, ow, oh));
		testTextures.Add(MakeTest(TexFormat.R16f,         on16, off16, w,  h ));
		testTextures.Add(MakeTest(TexFormat.R16f,         on16, off16, ow, oh));
		testTextures.Add(MakeTest(TexFormat.R16u,         ushort.MaxValue, (ushort)0, w,  h ));
		testTextures.Add(MakeTest(TexFormat.R16u,         ushort.MaxValue, (ushort)0, ow, oh));
		testTextures.Add(MakeTest(TexFormat.R32,          1.0f, 0, w,  h ));
		testTextures.Add(MakeTest(TexFormat.R32,          1.0f, 0, ow, oh));

		Tests.Test(CheckTextureFormats);
		Tests.Test(CheckTextureRead);

		Tests.Screenshot("TexFormats.jpg", 0, 400, 400, 50, V.XYZ(0,-0.15f, 0), V.XYZ(0,-0.15f,-0.5f) );
	}

	bool CheckTextureFormats()
	{
		foreach (FormatTest test in testTextures)
		{
			if (test.tex.AssetState < 0)
				return false;
		}
		return true;
	}

	bool CheckTextureRead()
	{
		foreach (FormatTest test in testTextures)
		{
			Tex texture = test.tex;
			if (test.tex.Format != TexFormat.Rgba32       &&
				test.tex.Format != TexFormat.Rgba32Linear &&
				test.tex.Format != TexFormat.Bgra32       &&
				test.tex.Format != TexFormat.Bgra32Linear &&
				test.tex.Format != TexFormat.R32)
				continue;

			if (texture.AssetState < 0)
				continue;

			Color32[] colors1 = texture.GetColors(1);
			Color32[] colors0 = texture.GetColors(0);
			if (colors0 == null || colors1 == null)
				return false;
		}
		return true;
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