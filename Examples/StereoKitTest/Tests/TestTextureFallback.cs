// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

using StereoKit;
using System.Collections.Generic;

class TestTextureFallback : ITest
{
	List<Material> materials = new List<Material>();

	public void Initialize()
	{
		// A blank texture with no data will always just use the loading
		// fallback texture.
		Material emptyMat = Material.Unlit.Copy();
		emptyMat[MatParamName.DiffuseTex] = new Tex();
		materials.Add(emptyMat);

		// We can override the fallback texture per-material, in case the
		// global fallback texture doesn't work well.
		Material fallbackMat = Material.Unlit.Copy();
		Tex fallbackTex = new Tex();
		fallbackTex.FallbackOverride = Tex.GenColor(new Color(1,0,1,1), 2, 2);
		fallbackMat[MatParamName.DiffuseTex] = fallbackTex;
		materials.Add(fallbackMat);

		// Textures that encounter an error while loading will use the global
		// error texture.
		Log.Info("Expected texture load failure:");
		Material errorMat = Material.Unlit.Copy();
		errorMat[MatParamName.DiffuseTex] = Tex.FromFile("file_that_doesnt_exist.png");
		materials.Add(errorMat);

	}

	public void Shutdown()
	{
	}

	public void Step()
	{
		for (int i = 0; i < materials.Count; i++)
		{
			float x = (i - (materials.Count-1)/2.0f) * 0.3f;
			Mesh.Quad.Draw(materials[i], Matrix.TRS(new Vec3(x,0,-0.5f), V.XYZ(0,180,0), 0.2f));
		}
		Tests.Screenshot("Tests/FallbackTextures.jpg", 400,200, new Vec3(0, 0, -0.25f), new Vec3(0, 0, -0.5f));
	}
}