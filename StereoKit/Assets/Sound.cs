using System;
using System.Runtime.InteropServices;
using System.Text;

namespace StereoKit
{
	/// <summary>This class represents a sound effect! Excellent for blips
	/// and bloops and little clips that you might play around your scene.
	/// Not great for long streams of audio like you might see in a podcast.
	/// Right now, this supports .wav, .mp3, and procedurally generated 
	/// noises!
	/// 
	/// On HoloLens 2, sounds are automatically processed on the HPU, freeing
	/// up the CPU for more of your app's code. To simulate this same effect
	/// on your development PC, you need to enable spatial sound on your
	/// audio endpoint. To do this, right click the speaker icon in your
	/// system tray, navigate to "Spatial sound", and choose "Windows Sonic
	/// for Headphones." For more information, visit 
	/// https://docs.microsoft.com/en-us/windows/win32/coreaudio/spatial-sound </summary>
	public class Sound : IAsset
	{
		internal IntPtr _inst;

		/// <summary>Gets or sets the unique identifier of this asset resource!
		/// This can be helpful for debugging, managine your assets, or finding
		/// them later on!</summary>
		public string Id
		{
			get => Marshal.PtrToStringAnsi(NativeAPI.sound_get_id(_inst));
			set => NativeAPI.sound_set_id(_inst, value);
		}

		/// <summary>This will return the total length of the sound in
		/// seconds.</summary>
		public float Duration { get => NativeAPI.sound_duration(_inst); }

		/// <summary>This will return the total number of audio samples used
		/// by the sound! StereoKit currently uses 48,000 samples per second
		/// for all audio.</summary>
		public int TotalSamples { get => (int)NativeAPI.sound_total_samples(_inst); }

		/// <summary>This is the maximum number of samples in the sound that
		/// are currently available for reading via ReadSamples! ReadSamples
		/// will reduce this number by the amount of samples read.
		/// 
		/// This is only really valid for Stream sounds, all other sound 
		/// types will just return 0.</summary>
		public int UnreadSamples { get => (int)NativeAPI.sound_unread_samples(_inst); }

		/// <summary>This is the current position of the playback cursor, 
		/// measured in samples from the start of the audio data.</summary>
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
			=> NativeAPI.sound_play(_inst, at, volume);

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

		/// <summary>This will read samples from the sound stream, starting
		/// from the first unread sample. Check UnreadSamples for how many
		/// samples are available to read.</summary>
		/// <param name="samples">A pre-allocated buffer to read the samples
		/// into! This function will stop reading when this buffer is full, 
		/// or when it runs out of unread samples.</param>
		/// <returns>Returns the number of samples that were read from the
		/// stream's buffer and written to the provided sample buffer.
		/// </returns>
		public int ReadSamples(ref float[] samples)
			=> (int)NativeAPI.sound_read_samples(_inst, samples, (ulong)samples.Length);

		/// <summary>Looks for a Sound asset that's already loaded, matching the given id!</summary>
		/// <param name="modelId">Which Sound are you looking for?</param>
		/// <returns>A link to the sound matching 'soundId', null if none is found.</returns>
		public static Sound Find(string modelId)
		{
			IntPtr sound = NativeAPI.sound_find(modelId);
			return sound == IntPtr.Zero ? null : new Sound(sound);
		}

		/// <summary>Loads a sound effect from file! Currently, StereoKit
		/// supports .wav and .mp3 files. Audio is converted to mono.</summary>
		/// <param name="filename">Name of the audio file! Supports .wav and
		/// .mp3 files.</param>
		/// <returns>A sound object, or null if something went wrong.</returns>
		public static Sound FromFile(string filename)
		{
			IntPtr inst = NativeAPI.sound_create(NativeHelper.ToUtf8(filename));
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
		{
			IntPtr inst = NativeAPI.sound_create_stream(streamBufferDuration);
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
		{
			IntPtr inst = NativeAPI.sound_create_samples(samplesAt48000s, (ulong)samplesAt48000s.Length);
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
			AudioGenerator tmpGen = generator;
			IntPtr inst = NativeAPI.sound_generate(tmpGen, duration);
			return inst == IntPtr.Zero ? null : new Sound(inst);
		}

		/// <inheritdoc cref="Default.SoundClick" />
		public static Sound Click => Default.SoundClick;
		/// <inheritdoc cref="Default.SoundUnclick" />
		public static Sound Unclick => Default.SoundUnclick;
	}
}
