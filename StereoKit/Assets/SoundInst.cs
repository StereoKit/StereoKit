using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>This represents a play instance of a Sound! You can get one
	/// when you call Sound.Play(). This allows you to do things like cancel
	/// a piece of audio early, or change the volume and position of it as 
	/// it's playing.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct SoundInst
	{
#pragma warning disable 0169 // handle is not "used", but required for interop
		ushort _id;
		short  _slot;
#pragma warning restore 0169

		/// <summary>The 3D position in world space this sound instance is
		/// currently playing at. If this instance is no longer valid, the
		/// position will be at zero.</summary>
		public Vec3 Position {
			get => NativeAPI.sound_inst_get_pos(this);
			set => NativeAPI.sound_inst_set_pos(this, value);
		}

		/// <summary>The volume multiplier of this Sound instance! Typically
		/// 0-1, where 0 is silent, and 1 is full volume. Values above 1
		/// amplify, and negatives clamp to 0.</summary>
		public float Volume {
			get => NativeAPI.sound_inst_get_volume(this);
			set => NativeAPI.sound_inst_set_volume(this, value);
		}

		/// <summary>Playback rate multiplier, clamped to 0.25-4. 1 is
		/// normal speed, 2 is twice as fast and an octave up. Animatable
		/// while playing.</summary>
		public float Pitch {
			get => NativeAPI.sound_inst_get_pitch(this);
			set => NativeAPI.sound_inst_set_pitch(this, value);
		}

		/// <summary>Apparent size of the source, 0-1. 0 is a point in
		/// space, 1 fills the whole sound field. Shaped emitters compute
		/// this themselves, treating a set value as their minimum.</summary>
		public float Spread {
			get => NativeAPI.sound_inst_get_spread(this);
			set => NativeAPI.sound_inst_set_spread(this, value);
		}

		/// <summary>Pause and resume this voice. A paused voice keeps its
		/// place and stays alive until stopped or stolen.</summary>
		public bool Paused {
			get => NativeAPI.sound_inst_get_paused(this);
			set => NativeAPI.sound_inst_set_paused(this, value);
		}

		/// <summary>This voice's playback position in source samples. For
		/// stream sounds this is an absolute position in the stream, so
		/// Sound.TotalSamples - Cursor is how much audio is queued ahead
		/// of this voice. Only fully in-memory sounds can Seek, streams
		/// read forward only.</summary>
		public ulong Cursor => NativeAPI.sound_inst_get_cursor(this);

		/// <summary>Jump this voice's playback to a sample position. Only
		/// works for fully in-memory sounds! Files up to ~10 seconds decode
		/// fully into memory on load, while longer files stream, and stream
		/// playback reads forward only.</summary>
		/// <param name="sample">Sample index to jump to, clamped to the
		/// sound's length.</param>
		public void Seek(ulong sample) => NativeAPI.sound_inst_seek(this, sample);

		/// <summary>Overrides the voice's low-pass filter cutoff in Hz,
		/// replacing the automatic distance model. 0 hands control back to
		/// the distance model.</summary>
		/// <param name="cutoffHz">Low-pass cutoff frequency in Hz, 0 for
		/// automatic.</param>
		public void SetCutoff(float cutoffHz) => NativeAPI.sound_inst_set_cutoff(this, cutoffHz);

		/// <summary>Gives this voice a polyline emitter shape! The emitter
		/// follows the listener along the shape - position becomes the
		/// closest point, apparent size grows as the shape fills more of
		/// the view, and the sound goes fully diffuse inside it. Great for
		/// streams, wind lines, and shorelines. Points are copied, max 32.
		/// </summary>
		/// <param name="points">The polyline's points, in world space.
		/// </param>
		/// <param name="radius">Radius of the polyline's tube, in meters.
		/// </param>
		public void SetShape(Vec3[] points, float radius)
			=> NativeAPI.sound_inst_set_shape(this, points, points.Length, radius);

		/// <summary>Gives this voice a sphere emitter shape! The emitter
		/// follows the listener around the sphere's surface, growing to
		/// fully diffuse inside it. Great for wind volumes and rain areas.
		/// </summary>
		/// <param name="center">The sphere's center, in world space.
		/// </param>
		/// <param name="radius">The sphere's radius, in meters.</param>
		public void SetShape(Vec3 center, float radius)
			=> NativeAPI.sound_inst_set_shape(this, new Vec3[] { center }, 1, radius);

		/// <summary>
		/// The maximum intensity of the sound data since the last frame, as a
		/// value from 0-1. This is unaffected by its 3d position or volume
		/// settings, and is straight from the audio file's data.
		/// </summary>
		public float Intensity => NativeAPI.sound_inst_get_intensity(this);

		/// <summary>Is this Sound instance currently playing? For streaming
		/// assets, this will be true even if they don't have any new data
		/// in them, and they're just idling at the end of their data.</summary>
		public bool IsPlaying => NativeAPI.sound_inst_is_playing(this);

		/// <summary>This stops the sound early if it's still playing.</summary>
		public void Stop() => NativeAPI.sound_inst_stop(this);
	}
}
