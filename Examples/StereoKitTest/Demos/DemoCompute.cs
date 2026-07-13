using System;
using System.Runtime.InteropServices;
using StereoKit;

class DemoCompute : ITest
{
	string title       = "Compute Shader";
	string description = "A reaction-diffusion simulation running entirely on the GPU via compute shaders, displayed on a quad.";

	[StructLayout(LayoutKind.Sequential)]
	struct Cell { public float A, B; }

	const int SimSize = 512;
	const int Groups  = SimSize / 8;

	Compute             computePing;
	Compute             computePong;
	ComputeBuffer<Cell> bufferA;
	ComputeBuffer<Cell> bufferB;
	Tex                 output;
	Material            quadMat;
	Pose                windowPose;
	int                 iteration;

	float simFeed     = 0.02f;
	float simKill     = 0.055f;
	float simDiffuseA = 0.2097f;
	float simDiffuseB = 0.105f;
	float simTimestep = 0.8f;
	int   stepsPerFrame = 2;

	public void Initialize()
	{
		Shader computeShader = Shader.FromFile("Shaders/compute_reaction.hlsl");
		computePing = new Compute(computeShader);
		computePong = new Compute(computeShader);

		bufferA = new ComputeBuffer<Cell>(ComputeBufferType.ReadWrite, SimSize * SimSize);
		bufferB = new ComputeBuffer<Cell>(ComputeBufferType.ReadWrite, SimSize * SimSize);
		ResetSim();

		output = new Tex(TexType.ImageNomips | TexType.Compute, TexFormat.Rgba32Linear);
		output.SetSize(SimSize, SimSize);

		// Ping: read A -> write B
		computePing.SetStorage("input",   bufferA);
		computePing.SetStorage("output",  bufferB);
		computePing.SetTexture("out_tex", output);

		// Pong: read B -> write A
		computePong.SetStorage("input",   bufferB);
		computePong.SetStorage("output",  bufferA);
		computePong.SetTexture("out_tex", output);

		UpdateSimParams();

		quadMat = new Material(Shader.Unlit);
		quadMat[MatParamName.DiffuseTex] = output;

		windowPose = (Demo.contentPose * Matrix.T(0.16f, 0.15f, 0)).Pose;
	}

	public void Step()
	{
		for (int i = 0; i < stepsPerFrame; i++)
		{
			Compute current = (iteration % 2 == 0) ? computePing : computePong;
			current.Dispatch((uint)Groups, (uint)Groups, 1);
			iteration++;
		}

		Vec3 at = Demo.contentPose.Transform(V.XYZ(0.16f, 0, 0));
		Mesh.Quad.Draw(quadMat, Matrix.TRS(at, Quat.FromAngles(0, 180, 0), 0.3f));

		UI.WindowBegin("Compute Settings", ref windowPose, new Vec2(0.3f, 0));
		bool changed = false;
		Vec2 size = new Vec2(0.08f, UI.LineHeight);
		UI.Label("Feed", size); UI.SameLine();
		changed |= UI.HSlider("Feed",     ref simFeed,     0.01f, 0.08f, 0.001f);
		UI.Label("Kill", size); UI.SameLine();
		changed |= UI.HSlider("Kill",     ref simKill,     0.03f, 0.08f, 0.001f);
		UI.Label("DiffuseA", size); UI.SameLine();
		changed |= UI.HSlider("DiffuseA", ref simDiffuseA, 0.05f, 0.40f, 0.001f);
		UI.Label("DiffuseB", size); UI.SameLine();
		changed |= UI.HSlider("DiffuseB", ref simDiffuseB, 0.02f, 0.20f, 0.001f);
		UI.Label("Timestep", size); UI.SameLine();
		changed |= UI.HSlider("Timestep", ref simTimestep, 0.1f,  2.0f,  0.1f);
		if (changed) UpdateSimParams();
		if (UI.Button("Reset")) ResetSim();
		UI.WindowEnd();

		Demo.ShowSummary(title, description, new Bounds(0.4f, 0.4f, 0.1f));
	}

	public void Shutdown() { }

	void ResetSim()
	{
		// Seed with block-based random patches. The reaction-diffusion
		// model is sensitive to initial conditions: A~random, B~(1-A)
		// in coherent 16x16 blocks.
		Cell[] initialData = new Cell[SimSize * SimSize];
		for (int y = 0; y < SimSize; y++)
		for (int x = 0; x < SimSize; x++)
		{
			float r = HashFloat(1, (uint)((x / 16) * 13 + (y / 16) * 127));
			initialData[x + y * SimSize] = new Cell { A = r, B = 1.0f - r };
		}
		bufferA.SetData(initialData);
		bufferB.SetData(initialData);
		iteration = 0;
	}

	void UpdateSimParams()
	{
		foreach (var c in new[] { computePing, computePong })
		{
			c["feed"]     = simFeed;
			c["kill"]     = simKill;
			c["diffuseA"] = simDiffuseA;
			c["diffuseB"] = simDiffuseB;
			c["timestep"] = simTimestep;
			c["size"]     = (uint)SimSize;
		}
	}

	static float HashFloat(int position, uint seed)
	{
		const uint BIT_NOISE1 = 0x68E31DA4;
		const uint BIT_NOISE2 = 0xB5297A4D;
		const uint BIT_NOISE3 = 0x1B56C4E9;

		uint mangled = (uint)position;
		mangled *= BIT_NOISE1;
		mangled += seed;
		mangled ^= (mangled >> 8);
		mangled += BIT_NOISE2;
		mangled ^= (mangled << 8);
		mangled *= BIT_NOISE3;
		mangled ^= (mangled >> 8);
		return (float)mangled / 4294967295.0f;
	}
}
