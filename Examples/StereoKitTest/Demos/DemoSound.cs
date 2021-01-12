using StereoKit;
using System;
using System.Collections.Generic;
using System.Numerics;

namespace StereoKitTest
{
	class DemoSound : ITest
	{
		Pose  windowPose = new Pose(.4f, 0, -0.4f, Quat.LookDir(-1,0,1));
		Sound fileSound  = Sound.FromFile("BlipNoise.wav");

		float      genDuration = 0.5f;
		Sound      genSound;
		Bounds     genVolume = new Bounds(new Vector3(0.25f, 0.25f, 0.25f));
		bool       genPrevContains = false;
		List<LinePoint> genPath = new List<LinePoint>();

		Mesh boundsMesh = Mesh.GenerateCube(Vec3.One);
		Material boundsMat = Default.MaterialUI.Copy();

		public void Initialize() {
			boundsMat.Transparency = Transparency.Blend;
			boundsMat[MatParamName.ColorTint] = new Color(1, 1, 1, 0.25f);

			/// :CodeSample: Sound Sound.FromFile Sound.Play
			/// ### Basic usage
			Sound sound = Sound.FromFile("BlipNoise.wav");
			sound.Play(Vec3.Zero);
			/// :End:

			/// :CodeSample: Sound Sound.Generate
			/// ### Generating a sound
			/// Making a procedural sound is pretty straightforward! Here's
			/// an example of building a sound from two frequencies of sin
			/// wave.
			Sound genSound = Sound.Generate((t) =>
			{
				float band1 = SKMath.Sin(t * 523.25f * SKMath.Tau); // a 'C' tone
				float band2 = SKMath.Sin(t * 659.25f * SKMath.Tau); // an 'E' tone
				const float volume = 0.1f;
				return (band1*0.6f + band2*0.4f) * volume;
			}, 0.5f);
			genSound.Play(Vec3.Zero);
			/// :End:
		}
		public void Shutdown() { }

		public void Update()
		{
			UI.WindowBegin("Sound", ref windowPose, new Vector2(20, 0) * U.cm);
			if (UI.Button("BlipNoise.wav"))
				fileSound.Play(windowPose.position);
			UI.Label("Generated Sound:");
			UI.Label("Duration"); UI.SameLine();
			UI.HSlider("Duration", ref genDuration, 0.1f, 2, 0, 8 * U.cm);
			if (genSound != null && UI.Button("Play"))
				genSound.Play(windowPose.position);
			UI.WindowEnd();

			boundsMesh.Draw(boundsMat, Matrix.TS(genVolume.center, genVolume.dimensions));

			Hand hand = Input.Hand(Handed.Right);
			bool contains = genVolume.Contains(hand[FingerId.Index, JointId.Tip].position);
			if (contains && !genPrevContains)
				genPath.Clear();
			if (contains) {
				Vector3 pt = hand[FingerId.Index, JointId.Tip].position - genVolume.center;
				if (genPath.Count == 0 || Vec3.DistanceSq(pt, genPath[genPath.Count-1].pt) > 0.0001f) {
					Vector3 rgb = (pt + (genVolume.dimensions / 2)) / genVolume.dimensions.X;
					genPath.Add(new LinePoint(pt, new Color(rgb.X, rgb.Y, rgb.Z), 0.01f));
				}
			}

			if (!contains && genPrevContains)
			{
				double band1=0, band2=0, tp=0;
				genSound = Sound.Generate((t) =>
				{
					double e = t - tp;
					tp = t;

					float   sampleAt = (t / genDuration) * (genPath.Count-1);
					float   pct      = sampleAt - (int)sampleAt;
					int     s1       = (int)sampleAt;
					int     s2       = (int)Math.Ceiling(sampleAt);
					Vector3 sample   = (Vector3.Lerp(genPath[s1].pt, genPath[s2].pt, pct) + genVolume.dimensions / 2) / genVolume.dimensions.X;

					band1 += e * (600 + sample.X * 4000) * 6.28f;
					band2 += e * (100 + sample.Z * 600 ) * 6.28f;

					return (float)(Math.Sin(band1)*0.3 + Math.Sin(band2)*0.3) * sample.Y;
				}, genDuration);
				genSound.Play(genVolume.center);
			}
			Lines.Add(genPath.ToArray());
			genPrevContains = contains;
		}
	}
}
