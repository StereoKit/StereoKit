// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

using StereoKit;
using System;
using System.Runtime.InteropServices;

/// <summary>A blind-pointing test for audio spatialization: a click
/// train plays from a hidden random direction, and you point at it and
/// pinch. Angular error is tracked across rounds, and the old bus-based
/// spatializer can be toggled live for an A/B comparison against the
/// per-voice direct binaural path. This is an interactive tool - the
/// headless test pass just idles.</summary>
class TestAudioAim : ITest
{
	// A/B hook exported by StereoKitC for spatializer listening tests.
	[DllImport("StereoKitC", CallingConvention = CallingConvention.Cdecl)]
	static extern void audio_test_force_bus(int enable);

	Pose      windowPose = (Demo.contentPose * Matrix.T(0, 0, 0)).Pose;
	Sound     clicks;
	SoundInst inst;
	Vec3      target;
	bool      roundActive;
	bool      busMode;
	float     resultTimer;
	float     lastError = -1;
	int       rounds;
	float     errorSum;
	Random    rand = new Random();

	public void Initialize()
	{
		// Broadband bursts localize far better than tones. For elevation:
		// every burst is the exact same waveform (cycle-local hash), since
		// the ear judges pinna notches against a known reference spectrum,
		// and the tilt is pink - white noise's treble excess reads as "up".
		float lp1 = 0, lp2 = 0;
		clicks = Sound.Generate((t) => {
			float cycle = t % 0.25f;
			if (cycle > 0.08f) { lp1 = 0; lp2 = 0; return 0; }
			uint  h = (uint)(cycle * 48000);
			h ^= h << 13; h ^= h >> 17; h ^= h << 5;
			float white = (h / (float)uint.MaxValue) * 2 - 1;
			lp1 += 0.25f * (white - lp1);
			lp2 += 0.05f * (white - lp2);
			float pink = white * 0.12f + lp1 * 0.25f + lp2 * 0.45f;
			// A sharp double-click: onset at 0, echo at 30ms - repetition
			// gives the ear structure to hang the spectral cues on.
			float env = MathF.Exp(cycle * -90);
			if (cycle >= 0.03f) env += MathF.Exp((cycle - 0.03f) * -90) * 0.7f;
			return pink * env;
		}, 1.0f);
		clicks.Decibels = 70;
	}

	void StartRound()
	{
		// A random direction around the head: full azimuth, and enough
		// elevation range to test up/down without getting unfair.
		float az = (float)rand.NextDouble() * MathF.PI * 2;
		float el = ((float)rand.NextDouble() * 2 - 1) * (60 * Units.deg2rad);
		Vec3  dir = new Vec3(
			MathF.Cos(el) * MathF.Sin(az),
			MathF.Sin(el),
			MathF.Cos(el) * MathF.Cos(az));
		target      = Input.Head.position + dir * 2.5f;
		inst        = clicks.Play(target, new SoundPlay { flags = SoundFlags.Loop });
		roundActive = true;
	}

	void EndRound(float error)
	{
		inst.Stop();
		roundActive = false;
		resultTimer = 3;
		if (error >= 0) {
			lastError = error;
			errorSum += error;
			rounds   += 1;
		}
	}

	public void Step()
	{
		// Pinch to lock in a guess - aimed with the hand ray, ignoring
		// pinches that are busy pressing UI.
		if (roundActive) {
			for (int h = 0; h < 2; h++) {
				Hand hand = Input.Hand((Handed)h);
				if (!hand.IsTracked || !hand.IsJustPinched) continue;
				if (Interactor.IsInteracting(h == 0 ? InteractorSource.HandLeft : InteractorSource.HandRight)) continue;

				Vec3  toTarget = (target - hand.aim.position).Normalized;
				float error    = Vec3.AngleBetween(hand.aim.Forward, toTarget);
				EndRound(error);
			}
		}

		// After a guess, reveal where it really was.
		if (resultTimer > 0) {
			resultTimer -= Time.Stepf;
			Lines.Add(Input.Head.position - Vec3.Up * 0.05f, target, new Color(0.5f, 1, 0.5f), 0.005f);
			Mesh.Sphere.Draw(Material.Default, Matrix.TS(target, 0.1f), new Color(0.5f, 1, 0.5f));
		}

		UI.WindowBegin("Audio Aim", ref windowPose);
		UI.PushEnabled(!Tests.IsTesting);
		if (rounds > 0)
			UI.Label($"Rounds: {rounds}  Last: {lastError:0.0}°  Avg: {errorSum / rounds:0.0}°");
		else
			UI.Label("Point at what you hear, pinch to guess!");

		if (roundActive) { if (UI.Button("Give up")) EndRound(-1); }
		else             { if (UI.Button("Start round")) StartRound(); }

		if (UI.Toggle("Classic bus spatializer", ref busMode))
			audio_test_force_bus(busMode ? 1 : 0);
		if (rounds > 0 && UI.Button("Reset stats")) { rounds = 0; errorSum = 0; lastError = -1; }
		UI.PopEnabled();
		UI.WindowEnd();
	}

	public void Shutdown()
	{
		if (roundActive) inst.Stop();
		audio_test_force_bus(0);
	}
}
