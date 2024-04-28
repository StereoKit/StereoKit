// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

using StereoKit;
using System;
using System.Collections.Generic;
using System.Numerics;
using System.Runtime.InteropServices;

internal class TestTextures : ITest
{
	struct FormatTest
	{
		public TexFormat format;
		public Tex       tex;
		public Material  mat;
	}
	List<FormatTest> testTextures = new();

	public void Initialize()
	{
		// 0x3C00 is a half float with value 1.0f
		ulong  on64  = 0x3C00000000003C00;
		ulong  off64 = 0x3C00000000000000;
		ushort on16  = 0x3C00;
		ushort off16 = 0x0000;
		ushort onRG  = 0xFF00;
		ushort offRG = 0x00FF;

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
		testTextures.Add(MakeTest(TexFormat.R8g8,         onRG, offRG, w,  h ));
		testTextures.Add(MakeTest(TexFormat.R8g8,         onRG, offRG, ow, oh));

		Tests.Test(CheckTextureFormats);
		Tests.Test(CheckTextureRead);
		Tests.Test(CheckBlankTex);
		Tests.Test(CheckBlockingSize);

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
		// RGBA values
		foreach (FormatTest test in testTextures)
		{
			Tex texture = test.tex;
			if (texture.AssetState < 0) continue;

			switch (test.tex.Format)
			{
				case TexFormat.Rgba32:
				case TexFormat.Rgba32Linear:
				case TexFormat.Bgra32:
				case TexFormat.Bgra32Linear:
					Color32[] colors1 = texture.GetColorData<Color32>(1);
					Color32[] colors0 = texture.GetColorData<Color32>(0);
					if (colors0 == null || colors1 == null)
						return false;
					break;
				case TexFormat.Rgba128:
					Color[] colors1_128 = texture.GetColorData<Color>(1);
					Color[] colors0_128 = texture.GetColorData<Color>(0);
					if (colors0_128 == null || colors1_128 == null)
						return false;
					break;
				case TexFormat.R8:
					byte[] colors1b = texture.GetColorData<byte>(1);
					byte[] colors0b = texture.GetColorData<byte>(0);
					if (colors0b == null || colors1b == null)
						return false;
					break;
				case TexFormat.R16f:
				case TexFormat.R16u:
					ushort[] colors1s = texture.GetColorData<ushort>(1);
					ushort[] colors0s = texture.GetColorData<ushort>(0);
					if (colors0s == null || colors1s == null)
						return false;
					break;
				case TexFormat.R32:
					float[] colors1f = texture.GetColorData<float>(1);
					float[] colors0f = texture.GetColorData<float>(0);
					if (colors0f == null || colors1f == null)
						return false;
					break;
				case TexFormat.Rgba64f:
					ushort[] colors1l = texture.GetColorData<ushort>(1, 4);
					ushort[] colors0l = texture.GetColorData<ushort>(0, 4);
					if (colors0l == null || colors1l == null)
						return false;
					break;
				case TexFormat.R8g8:
					byte[] colors1b2 = texture.GetColorData<byte>(1, 2);
					byte[] colors0b2 = texture.GetColorData<byte>(0, 2);
					if (colors0b2 == null || colors1b2 == null)
						return false;
					break;
			}
		}
		return true;
	}

	bool CheckBlankTex()
	{
		// A blank texture with no data on it should report a size of 0 and an
		// asset state of none. This verifies those things, and checks to
		// ensure Width/Height reads don't block in this case.

		Tex blank = new Tex();
		if (blank.Width != 0 || blank.Height != 0)
			return false;
		if (blank.AssetState != AssetState.None)
			return false;
		return true;
	}

	bool CheckBlockingSize()
	{
		// Tex.Width/Height will block execution and wait for texture metadata
		// to become available. This just checks to make sure it properly
		// surfaces the right value when it does.

		// Use FromMem to prevent this from pulling an existing asset.
		Tex tex = Tex.FromMemory(Platform.ReadFileBytes("test.png"));
		if (tex.Width != 256 || tex.Height != 256)
			return false;

		return true;
	}

	public void Step()
	{
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