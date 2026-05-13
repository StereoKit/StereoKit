// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith
// Copyright (c) 2026 Qualcomm Technologies, Inc.

using StereoKit;
using System;

class DemoHaptics : ITest
{
	string title       = "Controller Haptics";
	string description = "StereoKit's haptic API drives controller vibration in three modes: a simple Pulse that works on every controller, full PCM Waveforms for textured feel, and amplitude Curves for shaping intensity over time. The Procedural toggle drives the right controller's haptic output from its own velocity, like the wand-velocity sound demo.";

	Pose windowPose      = Demo.contentPose.Pose;
	InputHaptic active   = InputHaptic.RController;
	bool        procedural = false;

	// Per-frame buffer used by the procedural mode. Reused across calls to
	// avoid allocating during the inner loop.
	float[] procBuffer    = new float[256];
	Vec3    lastGripPos   = Vec3.Zero;
	bool    haveLastPos   = false;

	public void Initialize() { }
	public void Shutdown()
	{
		Input.HapticStop(InputHaptic.LController);
		Input.HapticStop(InputHaptic.RController);
	}

	public void Step()
	{
		if (procedural) StepProcedural(active);

		UI.WindowBegin("Haptics", ref windowPose, new Vec2(0.4f, 0));

		if (UI.Radio("Left",  active == InputHaptic.LController)) { active = InputHaptic.LController; Input.HapticStop(InputHaptic.RController); }
		UI.SameLine();
		if (UI.Radio("Right", active == InputHaptic.RController)) { active = InputHaptic.RController; Input.HapticStop(InputHaptic.LController); }

		InputHapticCaps caps = Input.HapticCaps(active);
		float           rate = Input.HapticPreferredRate(active);

		UI.Label($"Caps: {caps}");
		UI.Label($"Preferred rate: {(rate > 0 ? rate.ToString("0") + " Hz" : "any")}");

		UI.HSeparator();

		// Mode 1: Pulse. Always available when any actuator is present.
		UI.PushEnabled((caps & InputHapticCaps.Pulse) != 0);
		if (UI.Button("Pulse"))
			Input.HapticPulse(active, 200, 0.6f, 0.1f);
		UI.PopEnabled();

		// Mode 2: PCM waveform. Builds a 0.2s 200Hz sine burst and submits it
		// at the device's preferred rate (or 4kHz as a sane default).
		UI.SameLine();
		UI.PushEnabled((caps & InputHapticCaps.Waveform) != 0);
		if (UI.Button("Waveform"))
		{
			float r       = rate > 0 ? rate : 4000;
			int   count   = (int)(r * 0.2f);
			float[] wave  = new float[count];
			for (int i = 0; i < count; i++)
				wave[i] = (float)Math.Sin(2 * Math.PI * 200.0 * i / r) * 0.8f;
			Input.HapticWaveform(active, wave, r);
		}
		UI.PopEnabled();

		// Mode 3: Amplitude envelope. 100 unsigned samples at 100Hz (1 second
		// total) shaping a fade-in / fade-out double pulse.
		UI.SameLine();
		UI.PushEnabled((caps & InputHapticCaps.Curve) != 0);
		if (UI.Button("Curve"))
		{
			float[] env = new float[100];
			for (int i = 0; i < 100; i++)
			{
				float t  = i / 100f;
				float p1 = MathF.Max(0, 1 - MathF.Abs((t - 0.25f) * 8));
				float p2 = MathF.Max(0, 1 - MathF.Abs((t - 0.75f) * 8));
				env[i]   = MathF.Min(1, p1 + p2);
			}
			Input.HapticCurve(active, env, 100);
		}
		UI.PopEnabled();

		UI.HSeparator();

		// Long-buffer test: 10 seconds of a chirp, all submitted in one call.
		// StereoKit chunks it internally and feeds the runtime over many frames.
		UI.PushEnabled((caps & InputHapticCaps.Waveform) != 0);
		if (UI.Button("10s chirp"))
		{
			float r        = rate > 0 ? rate : 4000;
			int   count    = (int)(r * 10);
			float[] chirp  = new float[count];
			for (int i = 0; i < count; i++)
			{
				float t  = i / r;
				float fr = 80 + 240 * (t / 10f);   // sweep 80 -> 320 Hz
				chirp[i] = MathF.Sin(2 * MathF.PI * fr * t) * 0.7f;
			}
			Input.HapticWaveform(active, chirp, r);
		}
		UI.PopEnabled();

		UI.SameLine();
		if (UI.Button("Stop"))
			Input.HapticStop(active);

		UI.HSeparator();

		if (UI.Toggle("Procedural (drive from velocity)", ref procedural))
		{
			haveLastPos = false;
			if (!procedural) Input.HapticStop(active);
		}

		UI.WindowEnd();

		Demo.ShowSummary(title, description, new Bounds(new Vec3(0, -0.27f, 0), new Vec3(0.5f, 0.7f, 0)));
	}

	/// :CodeSample: Input.HapticPulse Input.HapticWaveform Input.HapticCaps Input.HapticPreferredRate Input.HapticStop
	/// ### Driving haptics from controller velocity
	/// This shows how to map a continuous physical signal (here, the
	/// controller's grip-pose velocity) onto haptic output. There are two
	/// paths: a simple per-frame `HapticPulse` that works on every device,
	/// and a streaming `HapticWaveform` path that's used when
	/// `XR_FB_haptic_pcm` is available.
	void StepProcedural(InputHaptic output)
	{
		Handed   hand    = output == InputHaptic.LController ? Handed.Left : Handed.Right;
		InputPose pose   = output == InputHaptic.LController ? InputPose.LGrip : InputPose.RGrip;
		PoseState state  = Input.PoseState(pose);
		if (!state.IsTracked()) { haveLastPos = false; return; }

		Vec3 pos = Input.Pose(pose).position;
		if (!haveLastPos) { lastGripPos = pos; haveLastPos = true; return; }

		float speed   = (pos - lastGripPos).Length / Math.Max(0.001f, Time.Stepf);
		lastGripPos   = pos;
		float intensity = MathF.Min(1, speed / 2.0f); // ~2 m/s saturates

		InputHapticCaps caps = Input.HapticCaps(output);
		if ((caps & InputHapticCaps.Waveform) != 0)
		{
			// Streaming path: synthesize one frame's worth of samples at the
			// device's preferred rate, append onto the existing stream.
			float r     = Input.HapticPreferredRate(output);
			if (r <= 0) r = 4000;
			int   count = (int)(r * Time.Stepf);
			if (procBuffer.Length != count) procBuffer = new float[count];
			for (int i = 0; i < count; i++)
				procBuffer[i] = MathF.Sin(2 * MathF.PI * 220 * i / r) * intensity;
			Input.HapticWaveform(output, procBuffer, r, append: true);
		}
		else if ((caps & InputHapticCaps.Pulse) != 0)
		{
			// Fallback path: per-frame pulse with current intensity.
			Input.HapticPulse(output, 0, intensity, Time.Stepf);
		}
	}
	/// :End:
}
