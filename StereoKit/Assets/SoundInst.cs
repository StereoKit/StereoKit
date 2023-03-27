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

		/// <summary>The volume multiplier of this Sound instance! A number
		/// between 0 and 1, where 0 is silent, and 1 is full volume.</summary>
		public float Volume {
			get => NativeAPI.sound_inst_get_volume(this);
			set => NativeAPI.sound_inst_set_volume(this, value);
		}

		/// <summary>Is this Sound instance currently playing? For streaming
		/// assets, this will be true even if they don't have any new data
		/// in them, and they're just idling at the end of their data.</summary>
		public bool IsPlaying => NativeAPI.sound_inst_is_playing(this);

		/// <summary>This stops the sound early if it's still playing.</summary>
		public void Stop() => NativeAPI.sound_inst_stop(this);
	}
}
