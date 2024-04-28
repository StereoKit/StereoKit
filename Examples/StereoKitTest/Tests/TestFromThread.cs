// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

using StereoKit;
using System.Threading.Tasks;

class TestFromThread : ITest
{
	bool running;
	int  frames;

	Material uniqueShaderMat = null;
	Material material        = null;
	Mesh     mesh            = null;

	public void Initialize()
	{
		running = true;
		frames  = 1;
		Tests.RunForFrames(frames);

		// This tests a collection of GPU resource creation and modification
		// operations from a separate thread. This is something that OpenGL in
		// particular tends to have problems with, on PC this can be tested
		// using the SKG_FORCE_OPENGL preprocessor define.
		Task.Run(() =>
		{
			// Load the shader via memory to force load a unique new shader
			// with a brand new id.
			Shader uniqueshader = Shader.FromMemory(Platform.ReadFileBytes("Shaders/floor_shader.hlsl.sks"));
			uniqueShaderMat = new Material(uniqueshader);
			material        = Material.Default.Copy();
			mesh            = Mesh.GenerateSphere(1);

			Color32[] colors = new Color32[1] { Color.White };
			Tex tex = Tex.FromColors(colors, 1, 1);
			tex.SetColors(1, 1, colors);

			material[MatParamName.DiffuseTex] = tex;

			Log.Info("Loaded multithreaded data");
			running = false;
		});
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
		if (running) { frames += 1; Tests.RunForFrames(frames); }

		// Use some of the resources to force them through the GPU. This could
		// potentially catch some issues with the resources.
		if (mesh != null && material != null)
			mesh.Draw(material, Matrix.TS(0, 0, -1, 0.75f));
		if (uniqueShaderMat != null)
			Mesh.Quad.Draw(uniqueShaderMat, Matrix.TRS(V.XYZ(0,-0.5f,-1), Quat.FromAngles(90,0,0), 2));
	}
}