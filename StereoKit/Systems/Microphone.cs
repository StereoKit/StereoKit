using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	public static class Microphone
	{
		private static Sound micSound;

		public static Sound Sound       { get => micSound; }
		public static bool  IsRecording { get => NativeAPI.mic_is_recording(); }

		public static string[] GetDevices()
		{
			int      count  = NativeAPI.mic_device_count();
			string[] result = new string[count];
			for (int i = 0; i < count; i++)
				result[i] = Marshal.PtrToStringAnsi(NativeAPI.mic_device_name(i));

			return result;
		}
		public static bool Start(string deviceName = null)
		{
			IntPtr ptr = NativeAPI.mic_start(deviceName);
			if (ptr != IntPtr.Zero && micSound == null)
				micSound = new Sound(ptr);
			if (ptr == IntPtr.Zero)
				micSound = null;
			return micSound == null;
		}
		public static void Stop()
		{
			NativeAPI.mic_stop();
			micSound = null;
		}
	}
}
