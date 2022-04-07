using StereoKit;
using System;
using System.Collections.Generic;

namespace StereoKitTest
{
	class DemoSound : ITest
	{
		Pose  windowPose = new Pose(.4f, 0, -0.4f, Quat.LookDir(-1,0,1));
		Sound fileSound  = Sound.FromFile("BlipNoise.wav");

		float      genDuration = 0.5f;
		Sound      genSound;
		Bounds     genVolume = new Bounds(new Vec3(.5f, 0, 0), new Vec3(0.25f, 0.25f, 0.25f));
		bool       genPrevContains = false;
		List<LinePoint> genPath = new List<LinePoint>();

		Sound       wandStream;
		SoundInst   wandStreamInst;
		Pose        wandPose = new Pose(.5f, 0, -0.3f, Quat.LookDir(1,0,0));
		Model       wandModel;
		Vec3        wandTipPrev;
		LinePoint[] wandFollow = null;
		float[]     wandSamples = new float[0];
		double      wandTime = 0;
		float       wandIntensity;
		void StepWand()
		{
			if (wandStream == null) { wandStream = Sound.CreateStream(5f); wandStreamInst = wandStream.Play(wandTipPrev); }
			if (wandModel  == null) wandModel = Model.FromFile("Wand.glb", Shader.UI);
			if (wandFollow == null) { wandFollow = new LinePoint[10]; for (int i=0;i<wandFollow.Length;i+=1) wandFollow[i] = new LinePoint(Vec3.Zero, new Color(1,1,1,i/(float)wandFollow.Length), (i / (float)wandFollow.Length)*0.01f+0.001f); }

			UI.HandleBegin("wand", ref wandPose, wandModel.Bounds);
			wandModel.Draw(Matrix.Identity);
			UI.HandleEnd();

			Vec3  wandTip   = wandPose.ToMatrix() * (wandModel.Bounds.center + wandModel.Bounds.dimensions.y * 0.5f * Vec3.Up);
			Vec3  wandVel   = (wandTip - wandTipPrev) * Time.Elapsedf;
			float wandSpeed = wandVel.Magnitude*100;
			
			int count = Math.Max(0, (int)(0.1f*48000) - (wandStream.TotalSamples - wandStream.CursorSamples));
			if (wandSamples.Length < count)
				wandSamples = new float[count];
			for (int i = 0; i < count; i++)
			{
				wandIntensity = Math.Min(1, SKMath.Lerp(wandIntensity, wandSpeed, 0.001f));
				wandTime += (1 / 48000.0) * (30000 * wandIntensity + 2000);
				wandSamples[i] = (float)Math.Sin(wandTime) * wandIntensity;
			}

			wandStreamInst.Position = wandTip;
			wandStream.WriteSamples(wandSamples, count);

			for (int i = 0; i < wandFollow.Length-1; i++)
				wandFollow[i].pt = wandFollow[i+1].pt;
			wandFollow[wandFollow.Length-1].pt = wandTip;
			Lines.Add(wandFollow);
			wandTipPrev = wandTip;
		}

		public void Initialize() {
			/// :CodeSample: Sound Sound.FromFile Sound.Play
			/// ### Basic usage
			Sound sound = Sound.FromFile("BlipNoise.wav");
			sound.Play(Vec3.Zero);
			/// :End:

			/// :CodeSample: Sound Sound.Generate
			/// ### Generating a sound via generator
			/// Making a procedural sound is pretty straightforward! Here's
			/// an example of building a 500ms sound from two frequencies of
			/// sin wave.
			Sound genSound = Sound.Generate((t) =>
			{
				float band1 = SKMath.Sin(t * 523.25f * SKMath.Tau); // a 'C' tone
				float band2 = SKMath.Sin(t * 659.25f * SKMath.Tau); // an 'E' tone
				const float volume = 0.1f;
				return (band1*0.6f + band2*0.4f) * volume;
			}, 0.5f);
			genSound.Play(Vec3.Zero);
			/// :End:

			/// :CodeSample: Sound Sound.FromSamples
			/// ### Generating a sound via samples
			/// Making a procedural sound is pretty straightforward! Here's
			/// an example of building a 500ms sound from two frequencies of
			/// sin wave.
			float[] samples = new float[(int)(48000*0.5f)];
			for (int i = 0; i < samples.Length; i++)
			{
				float t = i/48000.0f;
				float band1 = SKMath.Sin(t * 523.25f * SKMath.Tau); // a 'C' tone
				float band2 = SKMath.Sin(t * 659.25f * SKMath.Tau); // an 'E' tone
				const float volume = 0.1f;
				samples[i] = (band1 * 0.6f + band2 * 0.4f) * volume;
			}
			Sound sampleSound = Sound.FromSamples(samples);
			sampleSound.Play(Vec3.Zero);
			/// :End:
		}
		public void Shutdown() { }

		public void Update()
		{
			StepWand();

			UI.WindowBegin("Sound", ref windowPose);
			if (UI.Button("BlipNoise.wav"))
				fileSound.Play(windowPose.position);
			UI.PanelBegin();
				UI.Label("Generated Sound:");
				UI.Label("Duration"); UI.SameLine();
				UI.HSlider("Duration", ref genDuration, 0.1f, 2, 0, 8 * U.cm);
				if (genSound != null && UI.Button("Play"))
					genSound.Play(windowPose.position);
			UI.PanelEnd();
			UI.WindowEnd();

			Default.MeshCube.Draw(Default.MaterialUIBox, Matrix.TS(genVolume.center, genVolume.dimensions));

			Hand hand     = Input.Hand(Handed.Right);
			bool contains = genVolume.Contains(hand[FingerId.Index, JointId.Tip].position);
			if (contains && !genPrevContains)
				genPath.Clear();
			if (contains) {
				Vec3 pt = hand[FingerId.Index, JointId.Tip].position;
				if (genPath.Count == 0 || Vec3.DistanceSq(pt, genPath[genPath.Count-1].pt) > 0.0001f) {
					Vec3 rgb = (pt + (genVolume.dimensions / 2)) / genVolume.dimensions.x;
					genPath.Add(new LinePoint(pt, new Color(rgb.x, rgb.y, rgb.z), 0.01f));
				}
			}

			if (Input.Key(Key.Space).IsJustActive())
				fileSound.Play(hand[FingerId.Index,JointId.Tip].position);

			if (!contains && genPrevContains)
			{
				double band1=0, band2=0, tp=0;
				genSound = Sound.Generate((t) =>
				{
					double e = t - tp;
					tp = t;

					float sampleAt = (t / genDuration) * (genPath.Count-1);
					float pct      = sampleAt - (int)sampleAt;
					int   s1       = (int)sampleAt;
					int   s2       = (int)Math.Ceiling(sampleAt);
					Vec3  sample   = (Vec3.Lerp(genPath[s1].pt, genPath[s2].pt, pct)-genVolume.center + genVolume.dimensions / 2) / genVolume.dimensions.x;

					band1 += e * (600 + sample.x * 4000) * 6.28f;
					band2 += e * (100 + sample.z * 600 ) * 6.28f;

					return (float)(Math.Sin(band1)*0.3 + Math.Sin(band2)*0.3) * sample.y * 0.5f;
				}, genDuration);
				genSound.Play(genVolume.center);
			}
			Lines.Add(genPath.ToArray());
			genPrevContains = contains;
		}
	}
}
