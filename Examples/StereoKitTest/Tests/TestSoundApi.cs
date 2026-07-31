using StereoKit;
using System;
using System.IO;

// Binding-surface checks for the audio revamp's C# API. Timing-dependent
// behavior lives in the native offline suite (SKTests -audiotest), these
// verify the marshaling and immediate main-thread semantics.
class TestSoundApi : ITest
{
	Sound memSound;
	int   frame = 0;

	static byte[] MakeWav(float seconds, int channels)
	{
		int frames = (int)(48000 * seconds);
		using MemoryStream stream = new MemoryStream();
		using BinaryWriter w      = new BinaryWriter(stream);
		w.Write("RIFF"u8); w.Write(36 + frames*channels*4); w.Write("WAVE"u8);
		w.Write("fmt "u8); w.Write(16);
		w.Write((short)3); w.Write((short)channels);
		w.Write(48000);    w.Write(48000 * channels * 4);
		w.Write((short)(channels*4)); w.Write((short)32);
		w.Write("data"u8); w.Write(frames*channels*4);
		for (int i = 0; i < frames; i++)
			for (int c = 0; c < channels; c++)
				w.Write(MathF.Sin(i / 48000.0f * 440 * 6.2831853f) * 0.25f);
		return stream.ToArray();
	}

	static bool TestPolyphony()
	{
		Sound sound = Sound.Generate(t => MathF.Sin(t * 440 * 6.2831853f) * 0.1f, 1);
		SoundInst a = sound.Play(new Vec3(0, 0, -1));
		SoundInst b = sound.Play(new Vec3(0, 0, -1));
		SoundInst c = sound.Play(new Vec3(0, 0, -1));
		bool ok = a.IsPlaying && b.IsPlaying && c.IsPlaying;
		a.Stop(); b.Stop(); c.Stop();
		return ok;
	}

	static bool TestPlaySettings()
	{
		Sound sound = Sound.Generate(t => MathF.Sin(t * 440 * 6.2831853f) * 0.1f, 1);

		// A shaped play resolves its emitter immediately on the main
		// thread: a 1m sphere at 2m subtends asin(0.5) -> spread ~0.33.
		SoundInst inst = sound.Play(Vec3.Zero, new SoundPlay {
			pitch       = 2,
			bus         = SoundBus.Music,
			shape       = new Vec3[] { new Vec3(0, 0, -2) },
			shapeRadius = 1,
		});
		bool ok = inst.IsPlaying;
		ok = ok && Math.Abs(inst.Pitch  - 2)     < 0.001f;
		ok = ok && Math.Abs(inst.Spread - 0.33f) < 0.05f;

		inst.Pitch  = 0.5f;
		inst.Paused = true;
		ok = ok && Math.Abs(inst.Pitch - 0.5f) < 0.001f && inst.Paused;

		inst.Stop();
		return ok;
	}

	static bool TestStreamWrite()
	{
		// Overflowing a full ring keeps the newest samples.
		Sound   stream = Sound.CreateStream(0.5f); // 24,000 sample capacity
		float[] ramp   = new float[30000];
		for (int i = 0; i < ramp.Length; i++) ramp[i] = i;
		stream.WriteSamples(ramp);

		float[] read  = new float[30000];
		int     count = stream.ReadSamples(ref read);
		return count == 24000 && read[0] == 6000 && read[count-1] == 29999;
	}

	static bool TestGenerateChannels()
	{
		// Stereo generation: left-only content, interleaved.
		Sound stereo = Sound.Generate((samples, start) => {
			for (int i = 0; i < samples.Length / 2; i++) {
				samples[i*2  ] = MathF.Sin((start + (ulong)i) / 48000.0f * 440 * 6.2831853f) * 0.2f;
				samples[i*2+1] = 0;
			}
		}, 0.25f, SoundChannels.Stereo);
		bool ok = stereo.Channels == SoundChannels.Stereo;
		ok = ok && Math.Abs(stereo.Duration - 0.25f) < 0.01f;

		// Ambisonic generation: a W-only omni bed.
		Sound bed = Sound.Generate((samples, start) => {
			for (int i = 0; i < samples.Length / 4; i++) {
				samples[i*4] = MathF.Sin((start + (ulong)i) / 48000.0f * 440 * 6.2831853f) * 0.2f;
				samples[i*4+1] = samples[i*4+2] = samples[i*4+3] = 0;
			}
		}, 0.25f, SoundChannels.Ambisonic1);
		ok = ok && bed.Channels == SoundChannels.Ambisonic1;

		// Interleaved FromSamples round-trips its format and count.
		float[] frames = new float[9600];
		Sound   from   = Sound.FromSamples(frames, SoundChannels.Stereo);
		ok = ok && from.Channels == SoundChannels.Stereo && from.TotalSamples == 9600;
		return ok;
	}

	static bool TestStreamFormats()
	{
		Sound speech = Sound.CreateStream(0.5f, SoundChannels.Mono, SoundSampleRate.Speech);
		speech.WriteSamples(new float[1600]);
		bool ok = Math.Abs(speech.Duration - 0.1f) < 0.01f;

		Sound stereo = Sound.CreateStream(0.5f, SoundChannels.Stereo);
		stereo.WriteSamples(new float[9600]); // 4800 frames interleaved
		ok = ok && stereo.Channels == SoundChannels.Stereo;
		ok = ok && stereo.UnreadSamples == 9600;
		return ok;
	}

	static bool TestAudioClass()
	{
		Audio.Volume = 0.5f;
		bool ok = Math.Abs(Audio.Volume - 0.5f) < 0.001f;
		Audio.Volume = 1;

		Audio.SetBusVolume(SoundBus.Ui, 0.25f);
		ok = ok && Math.Abs(Audio.GetBusVolume(SoundBus.Ui) - 0.25f) < 0.001f;
		Audio.SetBusVolume(SoundBus.Ui, 1);

		ok = ok && Audio.OutputDecibels >= -120;

		Audio.ListenerOverride = new Pose(0, 0, 1, Quat.Identity);
		ok = ok && Audio.ListenerOverride.HasValue;
		Audio.ListenerOverride = null;
		return ok;
	}

	public void Initialize()
	{
		Tests.Test(TestPolyphony);
		Tests.Test(TestPlaySettings);
		Tests.Test(TestStreamWrite);
		Tests.Test(TestGenerateChannels);
		Tests.Test(TestStreamFormats);
		Tests.Test(TestAudioClass);

		memSound = Sound.FromMemory(MakeWav(1, 2), "test/api_memwav");
		Tests.Test(() => memSound != null);
	}

	public void Step() => frame++;

	public void Shutdown()
	{
		// The async decode has had the scene's frames to finish: a 1s
		// stereo file, with its channel layout preserved.
		Tests.Test(() => memSound.Duration > 0.9f && memSound.Duration < 1.1f);
		Tests.Test(() => memSound.Channels == SoundChannels.Stereo);
	}
}
