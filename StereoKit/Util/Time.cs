namespace StereoKit
{
	/// <summary>This class contains time information for the current session and frame!</summary>
	public static class Time
	{
		/// <summary> How much time has elapsed since StereoKit was initialized? 64 bit time precision. </summary>
		public static double Total    { get { return NativeAPI.time_get     (); } }
		/// <summary> How much time has elapsed since StereoKit was initialized? 32 bit time precision. </summary>
		public static float  Totalf   { get { return NativeAPI.time_getf    (); } }
		/// <summary> How much time has elapsed since StereoKit was initialized? 64 bit time precision. 
		/// This version is unaffected by the Time.Scale value!</summary>
		public static double TotalUnscaled { get { return NativeAPI.time_get_unscaled(); } }
		/// <summary> How much time has elapsed since StereoKit was initialized? 32 bit time precision.
		/// This version is unaffected by the Time.Scale value!</summary>
		public static float  TotalUnscaledf { get { return NativeAPI.time_getf_unscaled(); } }
		/// <summary> How much time elapsed during the last frame? 64 bit time precision. </summary>
		public static double Elapsed  { get { return NativeAPI.time_elapsed (); } }
		/// <summary> How much time elapsed during the last frame? 32 bit time precision. </summary>
		public static float  Elapsedf { get { return NativeAPI.time_elapsedf(); } }
		/// <summary> How much time elapsed during the last frame? 64 bit time precision. 
		/// This version is unaffected by the Time.Scale value!</summary>
		public static double ElapsedUnscaled  { get { return NativeAPI.time_elapsed_unscaled (); } }
		/// <summary> How much time elapsed during the last frame? 32 bit time precision. 
		/// This version is unaffected by the Time.Scale value!</summary>
		public static float  ElapsedUnscaledf { get { return NativeAPI.time_elapsedf_unscaled(); } }
		/// <summary>Time is scaled by this value! Want time to pass slower? Set it to 0.5! Faster? Try 2!</summary>
		public static double Scale { set { NativeAPI.time_scale(value); } }

		/// <summary>This allows you to override the application time! The application
		/// will progress from this time using the current timescale.</summary>
		/// <param name="totalSeconds">What time should it now be? The app will progress from this point in time.</param>
		/// <param name="frameElapsedSeconds">How long was the previous frame? This is a number often used
		/// in motion calculations. If left to zero, it'll use the previous frame's time, and if the previous
		/// frame's time was also zero, it'll use 1/90.</param>
		public static void SetTime(double totalSeconds, double frameElapsedSeconds = 0)
			=> NativeAPI.time_set_time(totalSeconds, frameElapsedSeconds);
	}
}
