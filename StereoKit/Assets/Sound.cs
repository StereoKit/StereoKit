using System;
using System.Runtime.InteropServices;
using System.Text;

namespace StereoKit
{
	/// <summary>This class represents a sound effect! Excellent for blips
	/// and bloops and little clips that you might play around your scene.
	/// Right now, this supports .wav, .mp3, and procedurally generated
	/// noises!</summary>
	public class Sound : IAsset
	{
		internal IntPtr _inst;

		/// <summary>Gets or sets the unique identifier of this asset resource!
		/// This can be helpful for debugging, managing your assets, or finding
		/// them later on!</summary>
		public string Id
		{
			get => Marshal.PtrToStringAnsi(NativeAPI.sound_get_id(_inst));
			set => NativeAPI.sound_set_id(_inst, value);
		}

		/// <summary>This will return the total length of the sound in
		/// seconds.</summary>
		public float Duration { get => NativeAPI.sound_duration(_inst); }

		/// <summary>The sound's real-world loudness at 1 meter, in
		/// decibels! StereoKit measures the audio data's loudness, so the
		/// value you declare here is the loudness you get - the waveform is
		/// the *shape* of the sound, Decibels is how loud it is. Loudness
		/// then falls off physically with distance (-6dB per doubling), so
		/// louder things carry farther with no extra tuning.
		///
		/// Some reference points: rustling leaves 20, a whisper 30, calm
		/// conversation 60, a vacuum cleaner at arm's length 75, a busy
		/// street corner 80 (the default), shouting up close 88, a rock
		/// concert 110, thunder from a nearby strike 120.</summary>
		public float Decibels { get => NativeAPI.sound_get_decibels(_inst); set => NativeAPI.sound_set_decibels(_inst, value); }

		/// <summary>The channel format of this sound's data. Only Mono
		/// sounds spatialize - Stereo plays head-locked with its image
		/// intact, and Ambisonic1 is a world-fixed sound field that
		/// counter-rotates against the head.</summary>
		public SoundChannels Channels { get => NativeAPI.sound_get_channels(_inst); }

		/// <summary>Sounds loaded from file decode asynchronously - this
		/// tells you where that's at! Playing is safe at any point: a Play
		/// while still Loading is held until the data lands, then catches
		/// up as if it had started on time. Negative states mean the load
		/// failed, and any held plays die quietly.</summary>
		public AssetState AssetState => NativeAPI.sound_asset_state(_inst);

		/// <summary>This will return the total number of audio samples used
		/// by the sound! StereoKit currently uses 48,000 samples per second
		/// for all audio. For stream sounds this is everything ever
		/// written. Against a playing SoundInst.Cursor, the difference is
		/// how much audio is queued ahead of that voice's playback.
		/// </summary>
		public int TotalSamples { get => (int)NativeAPI.sound_total_samples(_inst); }

		/// <summary>This is the maximum number of samples in the sound that
		/// are currently available for reading via ReadSamples! ReadSamples
		/// will reduce this number by the amount of samples read. Playback
		/// doesn't consume samples - playing voices each keep their own
		/// cursor, see SoundInst.Cursor.
		///
		/// This is only really valid for Stream sounds, all other sound
		/// types will just return 0.</summary>
		public int UnreadSamples { get => (int)NativeAPI.sound_unread_samples(_inst); }

		/// <summary>How far ReadSamples has consumed into a stream sound,
		/// in samples. Playing voices don't move this - each tracks its own
		/// position in SoundInst.Cursor. Non-stream sounds return 0.
		/// </summary>
		public int CursorSamples { get => (int)NativeAPI.sound_cursor_samples(_inst); }

		internal Sound(IntPtr sound)
		{
			_inst = sound;
			if (_inst == IntPtr.Zero)
				Log.Err("Received an empty sound!");
		}
		/// <summary>Release reference to the StereoKit asset.</summary>
		~Sound()
		{
			if (_inst != IntPtr.Zero)
				NativeAPI.assets_releaseref_threadsafe(_inst);
		}

		/// <summary>Plays the sound at the 3D location specified, using the
		/// volume parameter as an additional volume control option! Sound
		/// volume falls off from 3D location, and can also indicate
		/// direction and location through spatial audio cues. So make sure
		/// the position is where you want people to think it's from!
		/// Currently, if this sound is playing somewhere else, it'll be
		/// canceled, and moved to this location.</summary>
		/// <param name="at">World space location for the audio to play at.
		/// </param>
		/// <param name="volume">Volume modifier for the effect! 1 means full
		/// volume, and 0 means completely silent.</param>
		/// <returns>Returns a link to the Sound's play instance, which you
		/// can use to track and modify how the sound plays after the initial
		/// conditions are set.</returns>
		public SoundInst Play(Vec3 at, float volume = 1)
			// A zeroed volume resolves to full trim natively, so preserve this
			// overload's documented 0 = silent with a near-silent value.
			=> Play(at, new SoundPlay { volume = volume == 0 ? 1e-8f : volume });

		/// <summary>Plays the sound at the 3D location specified, with
		/// extra settings! Pitch, onset delay, emitter shapes, bus routing,
		/// and behavior flags all live in SoundPlay - a default struct
		/// behaves just like the plain Play call.</summary>
		/// <param name="at">World space location for the audio to play at.
		/// Ignored for non-mono sounds and head-locked plays.</param>
		/// <param name="settings">Extra playback settings, see SoundPlay.
		/// </param>
		/// <returns>A link to the Sound's play instance for tracking and
		/// live adjustments.</returns>
		public SoundInst Play(Vec3 at, SoundPlay settings)
		{
			sound_play_t native = new sound_play_t {
				volume            = settings.volume,
				pitch             = settings.pitch,
				spread            = settings.spread,
				delay             = settings.delay,
				cutoff            = settings.cutoff,
				bus               = settings.bus,
				flags             = settings.flags,
				shape_point_count = settings.shape != null ? settings.shape.Length : 0,
				shape_radius      = settings.shapeRadius };

			if (native.shape_point_count == 0)
				return NativeAPI.sound_play(_inst, at, native);

			// Native copies the points during this call, so pinning the
			// managed array just for its duration is all it takes.
			GCHandle pin = GCHandle.Alloc(settings.shape, GCHandleType.Pinned);
			try
			{
				native.shape_points = pin.AddrOfPinnedObject();
				return NativeAPI.sound_play(_inst, at, native);
			}
			finally { pin.Free(); }
		}

		/// <summary>Only works if this Sound is a stream type! This writes
		/// a number of audio samples to the sample buffer, and samples 
		/// should be between -1 and +1. Streams are stored as ring buffers
		/// of a fixed size, so writing beyond the capacity of the ring
		/// buffer will overwrite the oldest samples.
		/// 
		/// StereoKit uses 48,000 samples per second of audio.</summary>
		/// <param name="samples">An array of audio samples, where each
		/// sample is between -1 and +1.</param>
		public void WriteSamples(in float[] samples)
			=> NativeAPI.sound_write_samples(_inst, samples, (ulong)samples.Length);
		/// <inheritdoc cref="WriteSamples(in float[])"/>
		/// <param name="sampleCount">You can use this to write only a subset
		/// of the samples in the array, rather than the entire array!</param>
		public void WriteSamples(in float[] samples, int sampleCount)
			=> NativeAPI.sound_write_samples(_inst, samples, (ulong)sampleCount);

		/// <summary>Only works if this Sound is a stream type! This writes
		/// a number of audio samples to the sample buffer, and samples 
		/// should be between -1 and +1. Streams are stored as ring buffers
		/// of a fixed size, so writing beyond the capacity of the ring
		/// buffer will overwrite the oldest samples.
		/// 
		/// StereoKit uses 48,000 samples per second of audio.
		/// 
		/// This variation of the method bypasses marshalling memory into C#,
		/// so it is the most optimal way to copy sound data if your source is
		/// already in native memory!</summary>
		/// <param name="samples">A pointer to a native array of `float` audio
		/// samples, where each sample is between -1 and +1.</param>
		/// <param name="sampleCount">You can use this to write only a subset
		/// of the samples in the array, rather than the entire array!</param>
		public void WriteSamples(IntPtr samples, int sampleCount)
			=> NativeAPI.sound_write_samples(_inst, samples, (ulong)sampleCount);

		/// <summary>This will read samples from the sound stream, starting
		/// from the first unread sample. Check UnreadSamples for how many
		/// samples are available to read.</summary>
		/// <param name="samples">A pre-allocated buffer to read the samples
		/// into! This function will stop reading when this buffer is full,
		/// or when the sound runs out of unread samples.</param>
		/// <returns>Returns the number of samples that were read from the
		/// stream's buffer and written to the provided sample buffer.
		/// </returns>
		public int ReadSamples(ref float[] samples)
			=> (int)NativeAPI.sound_read_samples(_inst, samples, (ulong)samples.Length);

		/// <summary>This will read samples from the sound stream, starting
		/// from the first unread sample. Check UnreadSamples for how many
		/// samples are available to read.</summary>
		/// <param name="sampleBuffer">A pointer to a pre-allocated native
		/// buffer of floats to read the samples into! This function will stop
		/// reading when this buffer is full, or when the sound runs out of
		/// unread samples.</param>
		/// <param name="sampleCount">The maximum number of samples to read,
		/// this should be less than or equal to the number of samples the
		/// sampleBuffer can contain.</param>
		/// <returns>Returns the number of samples that were read from the
		/// stream's buffer and written to the provided sample buffer.
		/// </returns>
		public int ReadSamples(IntPtr sampleBuffer, int sampleCount)
			=> (int)NativeAPI.sound_read_samples(_inst, sampleBuffer, (ulong)sampleCount);

		/// <summary>Looks for a Sound asset that's already loaded, matching the given id!</summary>
		/// <param name="soundId">Which Sound are you looking for?</param>
		/// <returns>A link to the sound matching 'soundId', null if none is found.</returns>
		public static Sound Find(string soundId)
		{
			IntPtr sound = NativeAPI.sound_find(soundId);
			return sound == IntPtr.Zero ? null : new Sound(sound);
		}

		/// <summary>Loads a sound from file! StereoKit supports .wav and
		/// .mp3 files. Mono sounds spatialize, stereo plays head-locked,
		/// and 4 channel files load as first order ambisonics: world-fixed
		/// sound fields that counter-rotate against the user's head, ideal
		/// for environmental beds like rain, wind, or crowds. Bare 4 channel
		/// content is read as ambiX (ACN order, SN3D - the YouTube 360
		/// convention), FuMa-tagged .amb files are converted on load, and
		/// other surround layouts downmix to stereo. Check Channels for what
		/// a file loaded as. Decoding happens asynchronously, but playing
		/// right away is fine - a Play before the decode finishes catches
		/// up to real time once it lands, as if it had started on schedule.
		/// </summary>
		/// <param name="filename">Name of the audio file! Supports .wav and
		/// .mp3 files.</param>
		/// <returns>A sound object, or null if the file isn't found.</returns>
		public static Sound FromFile(string filename)
		{
			IntPtr inst = NativeAPI.sound_create(filename);
			return inst == IntPtr.Zero ? null : new Sound(inst);
		}

		/// <summary>Loads a sound from a file's data in memory! Same format
		/// support and async decode behavior as FromFile. The data is
		/// copied, so the array is yours again as soon as this returns.
		/// </summary>
		/// <param name="data">The complete contents of an audio file.
		/// </param>
		/// <param name="id">A unique identifier for this sound - loading
		/// the same id again returns the already loaded sound.</param>
		/// <returns>A sound object, or null if something went wrong.</returns>
		public static Sound FromMemory(in byte[] data, string id)
		{
			IntPtr inst = NativeAPI.sound_create_mem(id, data, (UIntPtr)data.Length);
			return inst == IntPtr.Zero ? null : new Sound(inst);
		}

		/// <summary>Create a sound used for streaming audio in or out! This
		/// is useful for things like reading from a microphone stream, or
		/// playing audio from a source streaming over the network, or even
		/// procedural sounds that are generated on the fly!
		/// 
		/// Use stream sounds with the WriteSamples and ReadSamples 
		/// functions.</summary>
		/// <param name="streamBufferDuration">How much audio time should
		/// this stream be able to hold without writing back over itself?
		/// </param>
		/// <returns>A stream sound that can be read and written to.</returns>
		public static Sound CreateStream(float streamBufferDuration)
			=> CreateStream(streamBufferDuration, SoundChannels.Mono, SoundSampleRate.Default);

		/// <summary>Create a stream sound with an explicit channel format
		/// and sample rate! A 16,000hz mono stream suits speech pipelines,
		/// while a stereo stream can carry pre-rendered music. Written
		/// samples are interleaved for multi-channel formats, and playback
		/// resamples to the mixer's 48,000hz automatically.</summary>
		/// <param name="streamBufferDuration">How much audio time should
		/// this stream be able to hold without writing back over itself?
		/// </param>
		/// <param name="channels">The stream's channel format.</param>
		/// <param name="sampleRate">Capture/playback rate. SoundSampleRate
		/// names the common rates with notes - Default uses the mixer's native
		/// 48,000, Speech (16,000) suits speech pipelines. The enum value is
		/// the rate in Hz, so cast any integer rate to it for something off
		/// this list; playback resamples to 48,000 automatically.</param>
		/// <returns>A stream sound that can be read and written to.</returns>
		public static Sound CreateStream(float streamBufferDuration, SoundChannels channels, SoundSampleRate sampleRate = SoundSampleRate.Default)
		{
			IntPtr inst = NativeAPI.sound_create_stream(streamBufferDuration, channels, sampleRate);
			return inst == IntPtr.Zero ? null : new Sound(inst);
		}

		/// <summary>This function will create a sound from an array of
		/// samples. Values should range from -1 to +1, and there should be
		/// 48,000 values per second of audio.</summary>
		/// <param name="samplesAt48000s">Values should range from -1 to +1, 
		/// and there should be 48,000 per second of audio.</param>
		/// <returns>Returns a sound effect from the samples provided! Or
		/// null if something went wrong.</returns>
		public static Sound FromSamples(float[] samplesAt48000s)
			=> FromSamples(samplesAt48000s, SoundChannels.Mono);

		/// <summary>Create a sound from an array of samples with an
		/// explicit channel format! Multi-channel data is interleaved -
		/// stereo alternates left/right, and Ambisonic1 packs W,Y,Z,X per
		/// frame in the ambiX convention. 48,000 frames per second of
		/// audio.</summary>
		/// <param name="samplesAt48000s">Interleaved samples from -1 to +1,
		/// 48,000 frames per second.</param>
		/// <param name="channels">How the samples are laid out.</param>
		/// <returns>A sound effect from the samples provided! Or null if
		/// something went wrong.</returns>
		public static Sound FromSamples(float[] samplesAt48000s, SoundChannels channels)
		{
			IntPtr inst = NativeAPI.sound_create_samples(samplesAt48000s, (ulong)samplesAt48000s.Length, channels);
			return inst == IntPtr.Zero ? null : new Sound(inst);
		}

		/// <summary>This function will generate a sound from a function you
		/// provide! The function is called once for each sample in the
		/// duration. As an example, it may be called 48,000 times for each
		/// second of duration.</summary>
		/// <param name="generator">This function takes a time value as an
		/// argument, which will range from 0-duration, and should return a
		/// value from -1 - +1 representing the audio wave at that point in
		/// time.</param>
		/// <param name="duration">In seconds, how long should the sound be?
		/// </param>
		/// <returns>Returns a generated sound effect! Or null if something
		/// went wrong.</returns>
		public static Sound Generate(AudioGenerator generator, float duration)
		{
			// One native call total: the batch callback fills the whole
			// buffer by looping the per-sample generator in C#.
			AudioGeneratorBatch batch = (samples, start, count) => {
				float[] buffer = new float[count];
				for (ulong i = 0; i < count; i++)
					buffer[i] = generator((start + i) / 48000f);
				Marshal.Copy(buffer, 0, samples, (int)count);
			};
			IntPtr inst = NativeAPI.sound_generate(batch, duration, SoundChannels.Mono);
			GC.KeepAlive(batch);
			return inst == IntPtr.Zero ? null : new Sound(inst);
		}

		/// <summary>This function generates a sound by asking your function
		/// to fill whole buffers of samples! This is far faster than the
		/// per-sample overload, one interop call instead of one per sample.
		///
		/// With a channel format, the buffer holds frames-x-channels
		/// interleaved samples: stereo alternates left/right, and
		/// Ambisonic1 packs W,Y,Z,X per frame in the ambiX convention -
		/// so procedural head-tracked sound fields are just a generator
		/// away.</summary>
		/// <param name="generator">Fills the provided buffer completely
		/// with interleaved audio sample values from -1 to +1. The second
		/// parameter is the index of the buffer's first frame, at 48,000
		/// frames per second.</param>
		/// <param name="duration">In seconds, how long should the sound be?
		/// </param>
		/// <param name="channels">The channel format the generator fills.
		/// </param>
		/// <returns>Returns a generated sound effect! Or null if something
		/// went wrong.</returns>
		public static Sound Generate(AudioBufferGenerator generator, float duration, SoundChannels channels = SoundChannels.Mono)
		{
			int channelCount = channels == SoundChannels.Ambisonic1 ? 4
			                 : channels == SoundChannels.Stereo     ? 2 : 1;
			AudioGeneratorBatch batch = (samples, start, count) => {
				float[] buffer = new float[count * (ulong)channelCount];
				generator(buffer, start);
				Marshal.Copy(buffer, 0, samples, buffer.Length);
			};
			IntPtr inst = NativeAPI.sound_generate(batch, duration, channels);
			GC.KeepAlive(batch);
			return inst == IntPtr.Zero ? null : new Sound(inst);
		}

		/// <inheritdoc cref="Default.SoundClick" />
		public static Sound Click => Default.SoundClick;
		/// <inheritdoc cref="Default.SoundUnclick" />
		public static Sound Unclick => Default.SoundUnclick;
	}
}
