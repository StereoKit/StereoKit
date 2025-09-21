using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>This class provides access to the hardware's microphone, and
	/// stores it in a Sound stream. Start and Stop recording, and check the
	/// Sound property for the results! Remember to ensure your application
	/// has microphone permissions enabled!</summary>
	public static class Microphone
	{
		private static Sound micSound;

		/// <summary>This is the sound stream of the Microphone when it is
		/// recording. This Asset is created the first time it is accessed 
		/// via this property, or during Start, and will persist. It is 
		/// re-used for the Microphone stream if you start/stop/switch 
		/// devices.</summary>
		public static Sound Sound { get { 
			if (micSound == null) 
				micSound = new Sound(NativeAPI.mic_get_stream()); 
			return micSound; 
		} }
		/// <summary>Tells if the Microphone is currently recording audio.
		/// </summary>
		public static bool IsRecording => NB.Bool(NativeAPI.mic_is_recording());

		/// <summary>Constructs a list of valid Microphone devices attached
		/// to the system. These names can be passed into Start to select
		/// a specific device to record from. It's recommended to cache this
		/// list if you're using it frequently, as this list is constructed 
		/// each time you call it.
		/// 
		/// It's good to note that a user might occasionally plug or unplug
		/// microphone devices from their system, so this list may
		/// occasionally change.</summary>
		/// <returns>List of human readable microphone device names.</returns>
		public static string[] GetDevices()
		{
			int      count  = NativeAPI.mic_device_count();
			string[] result = new string[count];
			for (int i = 0; i < count; i++)
			{ unsafe {
				result[i] = NU8.Str(NativeAPI.mic_device_name(i));
			} }

			return result;
		}

		/// <summary>This begins recording audio from the Microphone! Audio
		/// is stored in Microphone.Sound as a stream of audio. If the
		/// Microphone is already recording with a different device, it will
		/// stop the previous recording and start again with the new device.
		/// 
		/// If null is provided as the device, then they system's default
		/// input device will be used. Some systems may not provide access
		/// to devices other than the system's default.
		/// 
		/// On Android, this function requires user approved permissions to
		/// use. If these permissions are not already granted when Start is
		/// called, but the permission is available to the app, then this call
		/// will request permission and fail. You can check Permission.State
		/// and explicitly manage the permission yourself if you want to avoid
		/// this function failing the first time.</summary>
		/// <param name="deviceName">The name of the microphone device to
		/// use, as seen in the GetDevices list. null will use the system's
		/// default device preference.</param>
		/// <returns>True if recording started successfully, false for 
		/// failure. This could fail if the app does not have mic permissions,
		/// or if the deviceName is for a mic that has since been unplugged.
		/// </returns>
		public static bool Start(string deviceName = null)
		{ unsafe {
			byte* deviceNamePtr = NU8.Bytes(deviceName);
			bool  result        = NB.Bool(NativeAPI.mic_start(deviceNamePtr));
			NU8.Free(deviceNamePtr);
			return result;
		} }

		/// <summary>If the Microphone is recording, this will stop it.
		/// </summary>
		public static void Stop()
			=> NativeAPI.mic_stop();
	}
}
