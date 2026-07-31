// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith

using StereoKit;

class DocSound : ITest
{
	public void Initialize()
	{
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

		/// :CodeSample: Audio.Environment AudioEnvironment
		/// ### Acoustic environments
		/// Spatial sounds can play inside an acoustic environment - a
		/// shared reverb and early reflections that carry a sense of space
		/// and absolute distance! The default is entirely off, which costs
		/// nothing, and is the right resting state for AR. Preset constants
		/// cover common spaces:
		Audio.Environment = AudioEnvironment.Forest;

		// Or start from a preset and adjust it to taste:
		AudioEnvironment env = AudioEnvironment.Room;
		env.decay   = 0.6f;
		env.reflect = 0.7f;
		Audio.Environment = env;
		/// :End:
		Audio.Environment = AudioEnvironment.Off;
	}

	public void Step    () { }
	public void Shutdown() { }
}
