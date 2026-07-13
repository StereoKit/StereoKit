using System;

namespace StereoKit
{
	/// <summary>This class contains time information for the current session
	/// and frame!</summary>
	public static class Time
	{
		/// <summary> How many seconds have elapsed since StereoKit was
		/// initialized? 64 bit time precision, calculated at the start of the
		/// frame. </summary>
		public static double Total => NativeAPI.time_total();
		/// <summary> How many seconds have elapsed since StereoKit was
		/// initialized? 32 bit time precision, calculated at the start of the
		/// frame. </summary>
		public static float  Totalf => NativeAPI.time_totalf();
		/// <summary> How many seconds have elapsed since StereoKit was
		/// initialized? 64 bit time precision, calculated at the start of the
		/// frame. This version is unaffected by the Time.Scale value!
		/// </summary>
		public static double TotalUnscaled => NativeAPI.time_total_unscaled();
		/// <summary> How many seconds have elapsed since StereoKit was
		/// initialized? 32 bit time precision, calculated at the start of the
		/// frame. This version is unaffected by the Time.Scale value!
		/// </summary>
		public static float  TotalUnscaledf => NativeAPI.time_totalf_unscaled();
		/// <summary>The number of frames/steps since the app started.
		/// </summary>
		public static ulong  Frame => NativeAPI.time_frame();

		/// <summary> How many seconds have elapsed since the last frame? 64
		/// bit time precision, calculated at the start of the frame.</summary>
		public static double Step => NativeAPI.time_step();
		/// <summary> How many seconds have elapsed since the last frame? 32
		/// bit time precision, calculated at the start of the frame.</summary>
		public static float  Stepf => NativeAPI.time_stepf();
		/// <summary> How many seconds have elapsed since the last frame? 64
		/// bit time precision, calculated at the start of the frame. This
		/// version is unaffected by the Time.Scale value! </summary>
		public static double StepUnscaled => NativeAPI.time_step_unscaled();
		/// <summary> How many seconds have elapsed since the last frame? 32
		/// bit time precision, calculated at the start of the frame. This
		/// version is unaffected by the Time.Scale value! </summary>
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

		/// <summary>Microseconds of CPU work for the renderer during the
		/// most recently completed frame. This measures wall-clock time
		/// from command buffer acquisition through queue submission,
		/// excluding any time spent waiting on GPU fences or vsync. This
		/// is useful for identifying CPU-side rendering bottlenecks such
		/// as draw call overhead or resource uploads. Returns 0 if timing
		/// data is not yet available (first few frames).</summary>
		public static ulong PerfCPUus => NativeAPI.time_perf_cpu_us();
		
		/// <summary>Microseconds the GPU spent executing rendering
		/// commands for the most recently completed frame. Measured via
		/// hardware timestamp queries at the top and bottom of the
		/// Vulkan pipeline, so this reflects actual GPU execution time
		/// independent of CPU pacing or vsync. Useful for identifying
		/// GPU-bound scenarios like expensive shaders or overdraw.
		/// Returns 0 if timing data is not yet available (first few
		/// frames).</summary>
		public static ulong PerfGPUus => NativeAPI.time_perf_gpu_us();
	}
}
