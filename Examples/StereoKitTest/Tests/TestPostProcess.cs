using StereoKit;
using System;

// Verifies tile-friendly post-processing: material validation, array-order
// chaining, the RenderSettings path through DrawNow and RenderTo, depth
// reads, and the global chain API.
class TestPostProcess : ITest
{
	Material   matInvert;   // c * -1 + 1
	Material   matHalf;     // c * 0.5
	Material   matFog;      // depth fog, fog_color red
	RenderList emptyList;
	Tex        targetA;
	Tex        targetB;
	Tex        targetFog;
	Tex        targetTo;
	int        frame;
	int        errorLogs;

	static Matrix cam  = Matrix.Identity;
	static Matrix proj = Matrix.Perspective(90, 1, 0.1f, 10f);

	void OnLog(LogLevel level, string text) { if (level == LogLevel.Error) errorLogs++; }

	static bool Near(byte a, byte b) => Math.Abs(a - b) <= 4;

	public void Initialize()
	{
		Log.Subscribe(OnLog);

		// RenderTo draws the primary list plus sky - a black background makes
		// the pixel math predictable.
		Renderer.EnableSky = false;

		Shader mathShader = Shader.FromFile("postfx_math.hlsl");
		matInvert = new Material(mathShader);
		matInvert["mul_color"] = new Vec3(-1, -1, -1);
		matInvert["add_color"] = new Vec3( 1,  1,  1);
		matHalf   = new Material(mathShader);
		matHalf["mul_color"] = new Vec3(0.5f, 0.5f, 0.5f);

		matFog = new Material("postfx_depth_fog.hlsl");
		matFog["fog_color"]   = new Vec3(1, 0, 0);
		matFog["fog_density"] = 0.5f;

		emptyList = new RenderList();
		targetA   = Tex.RenderTarget(64, 64);
		targetB   = Tex.RenderTarget(64, 64);
		targetFog = Tex.RenderTarget(64, 64);
		targetTo  = Tex.RenderTarget(64, 64);

		// Materials that aren't valid post-process effects are rejected
		// with an error, and don't join the chain.
		int before = errorLogs;
		Renderer.SetPostProcess(Material.Default);
		Tests.Test(() => errorLogs == before + 1);

		// Clearing the global chain is error-free, even when it's empty.
		Renderer.SetPostProcess();
		Tests.Test(() => errorLogs == before + 1);

		// GPU readbacks need the drawn frame to complete, so work is spread
		// across frames in Step.
		Tests.RunForFrames(14);
	}

	public void Shutdown()
	{
		Renderer.EnableSky = true;
		Log.Unsubscribe(OnLog);
	}

	public void Step()
	{
		switch (frame)
		{
			case 0:
				// Invert runs first, half second: black -> 1.0 -> 0.5,
				// which lands at ~188 sRGB-encoded.
				emptyList.DrawNow(targetA, cam, proj, new RenderSettings {
					postProcess = new Material[] { matInvert, matHalf } });
				break;
			case 3: {
				Color32[] px = targetA.GetColorData<Color32>();
				Log.Info($"postfx invert+half: {px[64*32+32]}");
				Tests.Test(() => Near(px[64*32+32].r, 188));

				// Reversed array order: black -> 0.0 -> inverted to 1.0 =
				// 255. The array is the chain, in order.
				emptyList.DrawNow(targetB, cam, proj, new RenderSettings {
					postProcess = new Material[] { matHalf, matInvert } });
			} break;
			case 6: {
				Color32[] px = targetB.GetColorData<Color32>();
				Log.Info($"postfx half+invert: {px[64*32+32]}");
				Tests.Test(() => Near(px[64*32+32].r, 255));

				// Depth fog reads the depth input attachment; a cleared
				// (far) depth buffer fogs fully to fog_color.
				emptyList.DrawNow(targetFog, cam, proj, new RenderSettings {
					postProcess = new Material[] { matFog } });
			} break;
			case 9: {
				Color32[] px = targetFog.GetColorData<Color32>();
				Log.Info($"postfx fog: {px[64*32+32]}");
				Tests.Test(() => px[64*32+32].r > 250 && px[64*32+32].g < 8 && px[64*32+32].b < 8);

				// RenderTo queues for the next frame, and takes the same
				// settings. Also sneak in an over-long chain - it errors,
				// but still renders the first two in the array.
				int before = errorLogs;
				Renderer.RenderTo(targetTo, cam, proj, new RenderSettings {
					postProcess = new Material[] { matInvert, matHalf, matFog } });
				Tests.Test(() => errorLogs == before + 1);
			} break;
			case 13: {
				Color32[] px = targetTo.GetColorData<Color32>();
				Log.Info($"postfx renderto: {px[64*32+32]}");
				Tests.Test(() => Near(px[64*32+32].r, 188));
			} break;
		}
		frame++;
	}
}
