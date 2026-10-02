// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

using StereoKit;
using System.Diagnostics;

// A scrolling picket fence, a moving marker, a per frame hue strip, and a 16
// cell frame counter. A repeated or skipped display frame shows as a hitch in
// the fence, a hop in the marker, or a gap in a camera capture of the screen.
// The unpaced toggle advances by the wall clock gap between steps instead of
// Time.Step, which is what the loop used to do.
class TestFramePacing : ITest
{
	const float widthPx  = 1600;
	const float heightPx = 900;
	const float markerPx = 32;
	const float marginPx = 24;

	Material  material;
	Pose      panelPose = new Pose(0.7f, 0.1f, -0.85f, Quat.LookDir(-0.6f, 0, 1));
	Stopwatch clock     = Stopwatch.StartNew();

	float  scrollPx;
	float  markerPos;
	int    frameIndex;
	double lastWallS = -1;
	float  pacedStepPx;
	float  wallStepPx;

	const float pxPerFrame = 8;
	const float periodPx   = 64;
	const float barPx      = 32;

	bool  unpaced;
	float hitchEvery = 0;   // frames, 0 is off
	float hitchMs    = 30;

	public void Initialize()
	{
		material          = new Material(Shader.FromFile("frame_pacing.hlsl"));
		material.FaceCull = Cull.None;
		material.SetVector("size_px", new Vec2(widthPx, heightPx));
	}

	public void Shutdown() { }

	public void Step()
	{
		frameIndex++;

		double now    = clock.Elapsed.TotalSeconds;
		float  wallDt = lastWallS < 0 ? Time.Stepf : (float)(now - lastWallS);
		lastWallS     = now;

		PresentStats stats   = Time.PerfPresent;
		float        refresh = Device.DisplayRefreshRate;

		// Speed is in pixels per displayed frame, so a correctly paced fence
		// moves the same distance every refresh
		float pxPerS = pxPerFrame * refresh;
		pacedStepPx  = pxPerS * Time.Stepf;
		wallStepPx   = pxPerS * wallDt;
		float step   = unpaced ? wallStepPx : pacedStepPx;
		float track  = widthPx - marginPx - markerPx;
		scrollPx     = (scrollPx  + step) % periodPx;
		markerPos    = (markerPos + step) % track;

		if (hitchEvery > 0 && frameIndex % (int)hitchEvery == 0)
			Spin(hitchMs);

		material.SetVector("fence", new Vec4(scrollPx, periodPx, barPx, marginPx + markerPos));
		material.SetInt   ("frame_index", frameIndex);
		Mesh.Quad.Draw(material, Matrix.TS(V.XYZ(-0.25f, 0.15f, -1.2f), V.XYZ(1.6f, 0.9f, 1)));

		UI.WindowBegin("Frame Pacing", ref panelPose, V.XY(0.34f, 0));

		float latencyMs = stats.latencyUs / 1000f;
		UI.Text($"Step: paced {pacedStepPx:0.00} px, wall {wallStepPx:0.00} px\n" +
		        $"Display: {refresh:0.##} Hz\n" +
		        $"Latency: {latencyMs:0.0} ms\n" +
		        $"Repeated: {stats.repeatCount} of {stats.sampleCount}\n" +
		        $"CPU {Time.PerfCPUus / 1000f:0.00} ms, GPU {Time.PerfGPUus / 1000f:0.00} ms");
		UI.HSeparator();

		UI.Toggle("Unpaced", ref unpaced);

		UI.HSeparator();
		UI.Label("Inject CPU hitch");
		Slider("Every N frames", "hitch_n",  ref hitchEvery, 0, 240, 1);
		Slider("Stall ms",       "hitch_ms", ref hitchMs,    1, 100, 1);

		UI.HSeparator();
		// Fullscreen is also what moves the run ahead cap from the swapchain's
		// depth to two frames. The window manager gets the final say, so read
		// the state back rather than tracking the request.
		AppWindow window     = AppWindow.Main;
		bool      fullscreen = window != null && window.Fullscreen;
		UI.PushEnabled(window != null);
		if (UI.Toggle("Fullscreen", ref fullscreen))
			window.RequestFullscreen(fullscreen);
		UI.PopEnabled();

		UI.WindowEnd();
	}

	static void Slider(string label, string id, ref float value, float min, float max, float step)
	{
		UI.Label(label, V.XY(0.12f, 0));
		UI.SameLine();
		UI.Label($"{value:0}", V.XY(0.03f, 0));
		UI.SameLine();
		UI.HSlider(id, ref value, min, max, step);
	}

	static void Spin(float ms)
	{
		long end = Stopwatch.GetTimestamp() + (long)(ms / 1000.0 * Stopwatch.Frequency);
		while (Stopwatch.GetTimestamp() < end) { }
	}
}
