// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKit;
using System;
using System.Collections.Generic;
using System.IO;

class DemoSound : ITest
{
	string title       = "Sound";
	string description = "Sounds in StereoKit are spatial by default! This scene has sounds loaded from file on a grabbable emitter - mono spatializes, stereo plays head-locked, and ambisonic files are world-fixed sound fields - plus a sound you draw with your finger, a live synth stream on the wand, and acoustic environments.";

	Pose windowPose = (Demo.contentPose * Matrix.T(0.3f, 0.2f, 0)).Pose;

	// Sounds loaded from file live on a grabbable, scalable sphere in the
	// scene. Under 10cm across it plays as a point source, larger than
	// that it becomes a volume emitter - scale it up and walk inside!
	Sound     sphereSound  = Sound.FromFile("BlipNoise.wav");
	Pose      spherePose   = (Demo.contentPose * Matrix.T(-0.2f, 0.02f, 0)).Pose;
	float     sphereScale  = 1;
	SoundInst sphereInst;
	bool      sphereLoop;
	Material  sphereMat;
	const float sphereRadius = 0.04f;

	void StepSoundSphere()
	{
		float radius = sphereRadius * sphereScale;
		bool  area   = radius * 2 >= 0.1f;

		bool grabbed = UI.HandleBegin("soundSphere", ref spherePose,
			new Bounds(Vec3.Zero, Vec3.One * (sphereRadius * 2)), ref sphereScale);

		float pulse = sphereInst.IsPlaying ? sphereInst.Intensity : 0;
		float alpha = (grabbed ? 0.45f : 0.3f) + pulse * 0.4f;
		Mesh.Sphere.Draw(sphereMat, Matrix.S(radius * 2), new Color(0.5f, 0.7f, 1, alpha));

		// The controls sit where the line to the user's head crosses the
		// sphere's surface, facing the user - inside the sphere they fight
		// the handle for grabs, and hide behind the blended surface.
		Vec3 headLocal = Hierarchy.ToLocal(Input.Head.position);
		if (headLocal.MagnitudeSq < 0.0001f) headLocal = Vec3.Forward;
		Vec3 surface     = headLocal.Normalized * (radius + 0.005f);
		Quat facing      = Quat.LookAt(surface, headLocal);
		// The pose is the window's top center, offset up to center the row.
		Pose optionsPose = new Pose(surface + facing * V.XYZ(0, 0.016f, 0), facing);
		UI.WindowBegin("sphereOptions", ref optionsPose, UIWin.Body, UIMove.None);
		if (UI.Toggle("Loop", ref sphereLoop, Sprite.ToggleOff, Sprite.ToggleOn, UIBtnLayout.CenterNoText))
		{
			if (sphereLoop) sphereInst = sphereSound.Play(spherePose.position, new SoundPlay { flags = SoundFlags.Loop, volume = FileTrim() });
			else            sphereInst.Stop();
		}
		UI.SameLine();
		UI.PushEnabled(!sphereLoop);
		if (UI.ButtonImg("Play", Sprite.ArrowRight, UIBtnLayout.CenterNoText))
			sphereInst = sphereSound.Play(spherePose.position, new SoundPlay { volume = FileTrim() });
		UI.PopEnabled();
		UI.SameLine();
		UI.Label(area ? $"Area {radius * 2 * 100:0}cm" : "Point");
		UI.WindowEnd();
		UI.HandleEnd();

		// Keep the voice glued to the sphere: a sphere shape when it's an
		// area, and a plain position - which clears the shape - when not.
		// Only mono spatializes, other formats ignore position entirely.
		if (sphereInst.IsPlaying && sphereSound.Channels == SoundChannels.Mono)
		{
			if (area) sphereInst.SetShape(spherePose.position, radius);
			else      sphereInst.Position = spherePose.position;
		}
	}

	// Major pentatonic in just intonation - the scale where nothing clashes.
	static readonly float[] pentatonic = { 1, 9/8f, 5/4f, 3/2f, 5/3f };
	float      genDuration = 0.5f;
	Sound      genSound;
	Bounds     genVolume = new Bounds( Demo.contentPose.Translation, new Vec3(0.25f, 0.25f, 0.25f));
	bool       genPrevDrawing = false;
	List<LinePoint> genPath = new List<LinePoint>();

	Sound       wandStream;
	SoundInst   wandStreamInst;
	Pose        wandPose = (Demo.contentPose * Matrix.T(-0.38f, -0.05f, 0)).Pose;
	Model       wandModel;
	Vec3        wandTipPrev;
	LinePoint[] wandFollow = null;
	float[]     wandSamples = new float[0];
	double      wandTime = 0;
	float       wandIntensity;
	void StepWand()
	{
		if (wandModel == null) wandModel = Model.FromFile("Wand.glb", Shader.UI);

		UI.HandleBegin("wand", ref wandPose, wandModel.Bounds);
		wandModel.Draw(Matrix.Identity);
		UI.HandleEnd();

		Vec3 wandTip = wandPose.ToMatrix() * (wandModel.Bounds.center + wandModel.Bounds.dimensions.y * 0.5f * Vec3.Up);
		// The trail seeds at the tip, so the first frame has no streak.
		if (wandFollow == null) { wandFollow = new LinePoint[10]; for (int i=0;i<wandFollow.Length;i+=1) wandFollow[i] = new LinePoint(wandTip, new Color(1,1,1,i/(float)wandFollow.Length), (i / (float)wandFollow.Length)*0.01f+0.001f); }
		// The stream is born at the tip, and the previous-tip seed matches
		// so the first frame has no teleport and no spurious velocity.
		if (wandStream == null) {
			wandStream     = Sound.CreateStream(5f);
			wandTipPrev    = wandTip;
			wandStreamInst = wandStream.Play(wandTip);
		}

		// Tip speed in m/s, a brisk ~3m/s swing reaches full intensity.
		Vec3  wandVel   = (wandTip - wandTipPrev) / Math.Max(0.001f, Time.Stepf);
		float wandSpeed = wandVel.Magnitude / 3;

		// Keep ~100ms queued ahead of the voice's playback position.
		int count = Math.Max(0, (int)(0.1f*48000) - (wandStream.TotalSamples - (int)wandStreamInst.Cursor));
		if (wandSamples.Length < count)
			wandSamples = new float[count];
		for (int i = 0; i < count; i++)
		{
			wandIntensity = Math.Min(1, SKMath.Lerp(wandIntensity, wandSpeed, 0.001f));
			// The whistle glides A3->A5 with swing speed, a mid range where
			// a sine stays soft on the ear.
			wandTime += (1 / 48000.0) * (220 + 660 * wandIntensity) * 6.28318;
			// Streams skip loudness normalization, so bake in the sine's √2
			// crest factor - full intensity then sits at the declared dB.
			wandSamples[i] = (float)Math.Sin(wandTime) * wandIntensity * 1.4142f;
		}

		wandStreamInst.Position = wandTip;
		wandStream.WriteSamples(wandSamples, count);

		for (int i = 0; i < wandFollow.Length-1; i++)
			wandFollow[i].pt = wandFollow[i+1].pt;
		wandFollow[wandFollow.Length-1].pt = wandTip;
		Lines.Add(wandFollow);
		wandTipPrev = wandTip;
	}

	static readonly (string name, AudioEnvironment env)[] envs = {
		("Off",    AudioEnvironment.Off   ),
		("Room",   AudioEnvironment.Room  ),
		("Hall",   AudioEnvironment.Hall  ),
		("Cave",   AudioEnvironment.Cave  ),
		("Forest", AudioEnvironment.Forest),
		("Field",  AudioEnvironment.Field ) };
	int envActive = 0;

	// The acoustic environment is global state, and the choice here sticks
	// around across scenes on purpose - hear it against the other demos.
	void StepEnvironment()
	{
		for (int i = 0; i < envs.Length; i++)
		{
			if (i % 3 != 0) UI.SameLine();
			if (UI.Radio(envs[i].name, envActive == i) && envActive != i)
			{
				envActive = i;
				Audio.Environment = envs[i].env;
			}
		}
	}

	string fileName     = "BlipNoise.wav";
	float  fileDecibels = 80;

	// The slider is a per-voice trim offset from the sound's declared
	// loudness - the asset keeps its truth, only this playback gets moved.
	float FileTrim() => MathF.Pow(10, (fileDecibels - sphereSound.Decibels) / 20);

	// Any sound file can go on the sphere - the loader sorts out what it
	// is, and the label reports the result.
	void StepSoundFile()
	{
		if (UI.Button("Open file..."))
			Platform.FilePicker(PickerMode.Open, file => {
				sphereInst.Stop();
				sphereSound  = Sound.FromFile(file);
				fileName     = Path.GetFileName(file);
				fileDecibels = sphereSound.Decibels;
				if (sphereLoop)
					sphereInst = sphereSound.Play(spherePose.position, new SoundPlay { flags = SoundFlags.Loop, volume = FileTrim() });
			}, null, ".wav", ".mp3");
		UI.SameLine();
		UI.Label(fileName);

		// Channels isn't meaningful until the async decode finishes, so the
		// label tracks the load state immediate-mode.
		string kind = sphereSound.AssetState switch {
			AssetState.Loaded => sphereSound.Channels switch {
				SoundChannels.Stereo     => "Stereo, plays head-locked",
				SoundChannels.Ambisonic1 => "Ambisonic, a world-fixed field",
				_                        => "Mono, spatializes on the sphere" },
			AssetState.Loading => "Loading...",
			_                  => "Failed to load!",
		};
		UI.Label(kind);

		UI.Label($"{fileDecibels:0}dB", V.XY(4 * U.cm, UI.LineHeight));
		UI.SameLine();
		if (UI.HSlider("Decibels", ref fileDecibels, 40, 110, 1, 8 * U.cm))
			sphereInst.Volume = FileTrim();
	}

	public void Initialize()
	{
		sphereMat = Material.Unlit.Copy();
		sphereMat.Transparency = Transparency.Blend;
		sphereMat.DepthWrite   = false;

		// The environment may have been set on a previous visit, keep the
		// radio selection truthful. Wet 0 is always Off no matter the other
		// fields; a custom value selects nothing.
		AudioEnvironment current = Audio.Environment;
		envActive = current.wet == 0
			? 0
			: Array.FindIndex(envs, e => e.env.Equals(current));
	}

	public void Shutdown()
	{
		sphereInst    .Stop();
		wandStreamInst.Stop();
	}

	public void Step()
	{
		StepSoundSphere();
		StepWand();

		UI.WindowBegin("Sound", ref windowPose);

		UI.Text("Sound File", Align.TopCenter);
		UI.PanelBegin();
		StepSoundFile();
		UI.PanelEnd();

		UI.Text("Draw a Sound", Align.TopCenter);
		UI.PanelBegin();
		UI.Text("Pinch and drag in the box to draw a sound!");
		UI.Label("Duration"); UI.SameLine();
		UI.HSlider("Duration", ref genDuration, 0.1f, 2, 0, 8 * U.cm);
		UI.PushEnabled(genSound != null);
		if (UI.Button("Play")) genSound.Play(genVolume.center);
		UI.PopEnabled();
		UI.PanelEnd();

		UI.Text("Environment", Align.TopCenter);
		UI.PanelBegin();
		StepEnvironment();
		UI.PanelEnd();

		UI.WindowEnd();

		Default.MeshCube.Draw(Default.MaterialUIBox, Matrix.TS(genVolume.center, genVolume.dimensions));

		Hand hand     = Input.Hand(Handed.Right);
		Vec3 tip      = hand[FingerId.Index, JointId.Tip].position;
		bool contains = genVolume.Contains(tip);
		bool drawing  = contains && hand.IsPinched;

		if (contains)
			Mesh.Sphere.Draw(Material.Unlit, Matrix.TS(tip, 0.01f), Color.White);

		if (drawing && !genPrevDrawing)
			genPath.Clear();
		if (drawing) {
			if (genPath.Count == 0 || Vec3.DistanceSq(tip, genPath[genPath.Count-1].pt) > 0.0001f) {
				Vec3 rgb = (tip + (genVolume.dimensions / 2)) / genVolume.dimensions.x;
				genPath.Add(new LinePoint(tip, new Color(rgb.x, rgb.y, rgb.z), 0.01f));
			}
		}

		if (!drawing && genPrevDrawing && genPath.Count > 1)
		{
			double phase = 0, freq = 0, tp = 0;
			genSound = Sound.Generate((t) =>
			{
				double e = t - tp;
				tp = t;

				float sampleAt = (t / genDuration) * (genPath.Count-1);
				float pct      = sampleAt - (int)sampleAt;
				int   s1       = (int)sampleAt;
				int   s2       = (int)Math.Ceiling(sampleAt);
				Vec3  sample   = (Vec3.Lerp(genPath[s1].pt, genPath[s2].pt, pct)-genVolume.center + genVolume.dimensions / 2) / genVolume.dimensions.x;

				// X picks from two octaves of pentatonic and the oscillator
				// glides to it, theremin style - the slew also smooths the
				// corners between recorded path points.
				int    note   = Math.Clamp((int)(sample.x * 10), 0, 9);
				double target = 220 * pentatonic[note % 5] * (1 << note / 5);
				if (freq == 0) freq = target;
				freq  += (target - freq) * Math.Min(1, e * 200);
				phase += e * freq * 6.28318;

				// Z blends in harmonics: the back of the box is a pure dark
				// tone, up close to the user is bright.
				float bright = sample.z;
				float wave   = (float)(Math.Sin(phase)
					+ Math.Sin(phase * 2) * 0.45 * bright
					+ Math.Sin(phase * 3) * 0.22 * bright);

				// A 10ms attack/release window - without it the waveform
				// starts and ends on a step, an audible click at both edges.
				float envelope = Math.Min(1, Math.Min(t, genDuration - t) / 0.01f);

				return wave * sample.y * 0.5f * envelope;
			}, genDuration);
			// Normalization makes declared loudness the only volume knob -
			// the default 80dB is loud-radio, this is conversation level.
			genSound.Decibels = 68;
			genSound.Play(genVolume.center);
		}
		Lines.Add(genPath.ToArray());
		genPrevDrawing = drawing;

		Demo.ShowSummary(title, description, new Bounds(V.XY0(-0.02f, 0), V.XYZ(.84f, .48f, .3f)));
	}
}
