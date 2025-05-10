namespace StereoKit
{
	/// <summary>This class describes the device that is running this
	/// application! It contains identifiers, capabilities, and a few
	/// adjustable settings here and there.</summary>
	public static class Device
	{
		/// <summary>What type of display is this? Most XR headsets will report
		/// stereo, but the Simulator will report flatscreen.</summary>
		public static DisplayType DisplayType => NativeAPI.device_display_get_type ();
		/// <summary>Allows you to set and get the current blend mode of the
		/// device! Setting this may not succeed if the blend mode is not
		/// valid.</summary>
		public static DisplayBlend DisplayBlend
		{
			get => NativeAPI.device_display_get_blend();
			set => NativeAPI.device_display_set_blend(value);
		}
		/// <summary>Tells if a particular blend mode is valid on this device.
		/// Some devices may be capable of more than one blend mode.</summary>
		/// <param name="blend">The blend mode to check agains! AnyTransparent
		/// will check against additive and blend.</param>
		/// <returns>True if valid, false if not.</returns>
		public static bool ValidBlend(DisplayBlend blend)
			=> NativeAPI.device_display_valid_blend(blend);

		/// <summary>How many times the display refreshes each second, you
		/// might frequently see this at values like 75hz, 90hz, or 120hz! This
		/// value may change on the fly, as some devices may artificially
		/// throttle the display rate to save power or increase timing
		/// consistency, such as in cases where the device is also recording
		/// the display as a video stream.</summary>
		public static float DisplayRefreshRate => NativeAPI.device_display_get_refresh_rate();
		/// <summary>How much time between each screen refresh, you might
		/// frequently see this at values like 13.3ms(75hz), 11.1ms(90hz), or
		/// 8.3ms(120hz)! This value may change on the fly, as some devices may
		/// artificially throttle the display rate to save power or increase
		/// timing consistency, such as in cases where the device is also
		/// recording the display as a video stream.</summary>
		public static float DisplayRefreshInterval => NativeAPI.device_display_get_refresh_interval();

		/// <summary>The tracking capabilities of this device! Is it 3DoF,
		/// rotation only? Or is it 6DoF, with positional tracking as well?
		/// Maybe it can't track at all!</summary>
		public static DeviceTracking Tracking => NativeAPI.device_get_tracking();
		/// <summary>This is the name of the active device! From OpenXR, this
		/// is the same as systemName from XrSystemProperties. The simulator 
		/// will say "Simulator".</summary>
		public static string Name => NativeHelper.FromUtf8(NativeAPI.device_get_name());
		/// <summary>This is the name of the OpenXR runtime that powers the
		/// current device! This can help you determine which implementation
		/// quirks to expect based on the codebase used. On the simulator, this
		/// will be "Simulator", and in other non-XR modes this will be "None".
		/// </summary>
		public static string Runtime => NativeHelper.FromUtf8(NativeAPI.device_get_runtime());
		/// <summary>The reported name of the GPU, this will differ between D3D
		/// and GL.</summary>
		public static string GPU => NativeHelper.FromUtf8(NativeAPI.device_get_gpu());

		/// <summary>Does the device we're on have eye tracking support present
		/// for input purposes? This is _not_ an indicator that the user has
		/// given the application permission to access this information. See
		/// `Input.Gaze` for how to use this data.</summary>
		public static bool HasEyeGaze => NativeAPI.device_has_eye_gaze();

		/// <summary>Tells if the device is capable of tracking hands. This
		/// does not tell if the user is actually using their hands for input,
		/// merely that it's possible to!</summary>
		public static bool HasHandTracking => NativeAPI.device_has_hand_tracking();
	}
}
