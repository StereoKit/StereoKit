using System;

namespace StereoKit
{
	/// <summary>This class contains time information for the current session
	/// and frame!</summary>
	public static class Time
	{
		/// <summary> How much time has elapsed since StereoKit was
		/// initialized? 64 bit time precision. </summary>
		public static double Total => NativeAPI.time_total();
		/// <summary> How much time has elapsed since StereoKit was
		/// initialized? 32 bit time precision. </summary>
		public static float  Totalf => NativeAPI.time_totalf();
		/// <summary> How much time has elapsed since StereoKit was
		/// initialized? 64 bit time precision. This version is unaffected by
		/// the Time.Scale value!</summary>
		public static double TotalUnscaled => NativeAPI.time_total_unscaled();
		/// <summary> How much time has elapsed since StereoKit was
		/// initialized? 32 bit time precision. This version is unaffected by
		/// the Time.Scale value!</summary>
		public static float  TotalUnscaledf => NativeAPI.time_totalf_unscaled();

		/// <summary> (Deprecated, use Time.Step) How much time elapsed during
		/// the last frame? 64 bit time precision.</summary>
		[Obsolete("Use Time.Step")]
		public static double Elapsed => NativeAPI.time_step();
		/// <summary> (Deprecated, use Time.Stepf) How much time elapsed during
		/// the last frame? 32 bit time precision.</summary>
		[Obsolete("Use Time.Stepf")]
		public static float  Elapsedf => NativeAPI.time_stepf();
		/// <summary> (Deprecated, use Time.StepUnscaled) How much time elapsed
		/// during the last frame? 64 bit time precision. This version is
		/// unaffected by the Time.Scale value!</summary>
		[Obsolete("Use Time.StepUnscaled")]
		public static double ElapsedUnscaled => NativeAPI.time_step_unscaled ();
		/// <summary> (Deprecated, use Time.StepUnscaledf) How much time
		/// elapsed during the last frame? 32 bit time precision. This version
		/// is unaffected by the Time.Scale value!</summary>
		[Obsolete("Use Time.StepUnscaledf")]
		public static float  ElapsedUnscaledf => NativeAPI.time_stepf_unscaled();

		/// <summary> How much time has elapsed since the last frame? 64 bit
		/// time precision.</summary>
		public static double Step => NativeAPI.time_step();
		/// <summary> How much time has elapsed since the last frame? 32 bit
		/// time precision.</summary>
		public static float  Stepf => NativeAPI.time_stepf();
		/// <summary> How much time has elapsed since the last frame? 64 bit
		/// time precision. This version is unaffected by the Time.Scale value!
		/// </summary>
		public static double StepUnscaled => NativeAPI.time_step_unscaled();
		/// <summary> How much time has elapsed since the last frame? 32 bit
		/// time precision. This version is unaffected by the Time.Scale value!
		/// </summary>
		public static float  StepUnscaledf => NativeAPI.time_stepf_unscaled();

		/// <summary>Time is scaled by this value! Want time to pass slower?
		/// Set it to 0.5! Faster? Try 2!</summary>
		public static double Scale { set { NativeAPI.time_scale(value); } }

		/// <summary>This allows you to override the application time! The
		/// application will progress from this time using the current
		/// timescale.</summary>
		/// <param name="totalSeconds">What time should it now be? The app will
		/// progress from this point in time.</param>
		/// <param name="frameElapsedSeconds">How long was the previous frame?
		/// This is a number often used in motion calculations. If left to
		/// zero, it'll use the previous frame's time, and if the previous
		/// frame's time was also zero, it'll use 1/90.</param>
		public static void SetTime(double totalSeconds, double frameElapsedSeconds = 0)
			=> NativeAPI.time_set_time(totalSeconds, frameElapsedSeconds);
	}
}
