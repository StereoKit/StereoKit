using StereoKit;
using System.Collections.Generic;

/// <summary>
/// This basically sets up a situation with massive overdraw on a set of quads
/// with different materials. This overdraw allows us to check the perf for
/// each material a little easier.
/// </summary>
class TestPerfTester : ITest
{
	Material[] mat;
	Mesh       mesh;

	const int   layers            = 10;
	const float spacing           = 0.0f;  // Z distance between each layer
	      float scale             = Tests.IsTesting ? 2 : 0.2f;     // Size of the quad we're rendering on
	const int   testDuration      = 8;    // Run the test for X frames
	const int   testDurationStart = 3;     // Discard the first X frames of a test

	int      testIdx   = 0;
	int      testFrame = 0;
	ulong    testGpuUs = 0;
	double   baselineFrameAvg = 0;
	int      frame = 0;

	public void Initialize()
	{
		Tex t = Tex.FromFile("floor.png");
		mesh = Mesh.GeneratePlane(Vec2.One, -Vec3.Forward, Vec3.Up, 0);

		// Default must be first — it sets the baseline that all others are
		// compared against in the output.
		List<Material> mats = new List<Material>();
		mats.Add(Material.Default.Copy());
		mats.Add(Material.Unlit.Copy());
		mats.Add(Material.UnlitClip.Copy());
		mats.Add(Material.PBR.Copy());
		mats.Add(Material.PBRClip.Copy());
		mats.Add(Material.UI.Copy());
		mats.Add(Material.UIBox.Copy());

		for (int i = 0; i < mats.Count; i++)
		{
			mats[i][MatParamName.DiffuseTex] = t;
			mats[i].DepthTest = DepthTest.Always;
		}
		mat = mats.ToArray();

		//mat = new Material[] { Material.PBR.Copy(), Material.Unlit.Copy() };
		//mat[0][MatParamName.DiffuseTex] = Tex.FromFile("floor.png");
		//mat[0].DepthTest = DepthTest.Always;
		//mat[1][MatParamName.DiffuseTex] = Tex.FromFile("C:\\Temp\\Large-2048.ktx2");
		//mat[1].DepthTest = DepthTest.Always;

		Assets.BlockForPriority(int.MaxValue);

		testFrame = testDuration;
		testIdx = -1;

		Renderer.EnableSky = false;
	}

	public void Shutdown()
	{
		Renderer.EnableSky = true;
	}

	public void Step()
	{
		if (Tests.IsTesting)
		{
			Tests.RunContinue();
			if (testIdx >= 0) {
				for (int i = 0; i < layers; i++)
					mesh.Draw(mat[testIdx], Matrix.TS(V.XYZ(0, 0, -0.5f + layers * -spacing + i * spacing), scale));
			}
			testFrame -= 1;
			if (testFrame < (testDuration-testDurationStart))
				testGpuUs += Time.PerfGPUus;
			if (testFrame == 0)
			{
				if (testIdx >= 0)
				{
					double elapsed  = testGpuUs / 1_000_000.0;
					double frameAvg = elapsed/(testDuration-testDurationStart);
					if (testIdx == 0) baselineFrameAvg = frameAvg;
					Log.Info($"{frameAvg * 1000:00.000}ms ({frameAvg/baselineFrameAvg:0.00}x default) Material #{testIdx} ({mat[testIdx].Shader.Id})");
				}
				testIdx  += 1;
				testFrame = testDuration;
				testGpuUs = 0;

				if (testIdx >= mat.Length)
				{
					Tests.RunStop();
					testIdx = 0;
				}
			}
		}
		else
		{
			// This could probably use some UI
			for (int x = 0; x < mat.Length; x++)
			{
				float xPos = -((mat.Length - 1) * scale * 0.5f) + (x - 1) * scale;
				for (int i = 0; i < layers; i++)
				{
					mesh.Draw(mat[x], Matrix.TS(V.XYZ(xPos, 0, -0.5f + layers * -spacing + i * spacing), scale));
				}
			}
		}

		frame += 1;
	}
}