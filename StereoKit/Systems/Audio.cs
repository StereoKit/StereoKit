using System;

namespace StereoKit
{
	/// <summary>Global audio system controls: the master volume, per-bus
	/// category volumes, listener overrides, and an output meter for
	/// checking your mix.</summary>
	public static class Audio
	{
		/// <summary>The master volume, a 0-1 trim over everything StereoKit
		/// plays. This is an app level control - the user's system volume
		/// sits below it.</summary>
		public static float Volume
		{
			get => NativeAPI.audio_get_volume();
			set => NativeAPI.audio_set_volume(value);
		}

		/// <summary>Sets a bus category's 0-1 volume trim. Every sound
		/// playing on that bus is affected, handy for sfx/music/ui sliders
		/// in a settings menu, or ducking a whole category.</summary>
		/// <param name="bus">The bus to adjust.</param>
		/// <param name="volume">0-1 volume trim for the bus.</param>
		public static void SetBusVolume(SoundBus bus, float volume)
			=> NativeAPI.audio_set_bus_volume(bus, volume);

		/// <summary>Gets a bus category's current 0-1 volume trim.</summary>
		/// <param name="bus">The bus to inspect.</param>
		/// <returns>The bus's 0-1 volume trim.</returns>
		public static float GetBusVolume(SoundBus bus)
			=> NativeAPI.audio_get_bus_volume(bus);

		/// <summary>RMS level of the last mixed audio block in dBFS, -120
		/// when silent. Useful for level meters, and for checking where
		/// your content sits relative to the limiter at 0.</summary>
		public static float OutputDecibels
			=> NativeAPI.audio_get_output_decibels();

		/// <summary>The acoustic environment that spatial sounds play in!
		/// This drives a shared reverb and early reflections that carry a
		/// sense of space and absolute distance. The default is fully off
		/// (wet 0), which costs nothing and never fights the real room's
		/// acoustics - the right resting state for AR. Assign a preset like
		/// AudioEnvironment.Hall - Off returns to dry, zero cost playback -
		/// or build custom values, perhaps starting from a preset.</summary>
		public static AudioEnvironment Environment
		{
			get => NativeAPI.audio_get_env();
			set => NativeAPI.audio_set_env(value);
		}

		private static Pose? listenerOverride = null;
		/// <summary>Normally the audio listener follows the user's head.
		/// Set this to hear the scene from somewhere else - a third person
		/// camera, or a remote avatar - and set it to null to give the ears
		/// back to the head.</summary>
		public static Pose? ListenerOverride
		{
			get => listenerOverride;
			set
			{
				listenerOverride = value;
				if (value.HasValue)
				{
					Pose pose = value.Value;
					NativeAPI.audio_set_listener(in pose);
				}
				else
					NativeAPI.audio_set_listener(IntPtr.Zero);
			}
		}
	}
}
