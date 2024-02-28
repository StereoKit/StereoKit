using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>A class for logging errors, warnings and information!
	/// Different levels of information can be filtered out, and supports
	/// coloration via &lt;~[colorCode]&gt; and &lt;~clr&gt; tags.
	/// 
	/// Text colors can be set with a tag, and reset back to default with
	/// &lt;~clr&gt;. Color codes are as follows:
	/// 
	/// | Dark | Bright | Description |
	/// |------|--------|-------------|
	/// | DARK | BRIGHT | DESCRIPTION |
	/// | blk  | BLK    | Black       |
	/// | red  | RED    | Red         |
	/// | grn  | GRN    | Green       |
	/// | ylw  | YLW    | Yellow      |
	/// | blu  | BLU    | Blue        |
	/// | mag  | MAG    | Magenta     |
	/// | cyn  | cyn    | Cyan        |
	/// | grn  | GRN    | Green       |
	/// | wht  | WHT    | White       |
	/// </summary>
	public static class Log
	{
		#region Console Color Setup
		const int  STD_OUTPUT_HANDLE = -11;
		const uint ENABLE_VIRTUAL_TERMINAL_PROCESSING = 4;

		[DllImport("kernel32.dll", SetLastError = true)]
		static extern IntPtr GetStdHandle(int nStdHandle);

		[DllImport("kernel32.dll")]
		static extern bool GetConsoleMode(IntPtr hConsoleHandle, out uint lpMode);

		[DllImport("kernel32.dll")]
		static extern bool SetConsoleMode(IntPtr hConsoleHandle, uint dwMode);
		internal static void SetupConsoleWin32()
		{
			if (!Environment.OSVersion.Platform.ToString().ToLower().Contains("win"))
				return;

			IntPtr handle = GetStdHandle(STD_OUTPUT_HANDLE);
			uint mode;
			GetConsoleMode(handle, out mode);
			mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			SetConsoleMode(handle, mode);
		}
		internal static void SetupConsole()
		{
			if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
				SetupConsoleWin32();
		}
		#endregion

		static Dictionary<LogCallback, LogCallbackData> callbacks = new Dictionary<LogCallback, LogCallbackData>();

		/// <summary>What's the lowest level of severity logs to display on
		/// the console? Default is LogLevel.Info. This property can safely
		/// be set before SK initialization.</summary>
		public static LogLevel Filter { set{ NativeLib.Load(); SetFilter(value); } }
		private static void SetFilter(LogLevel level)
			=> NativeAPI.log_set_filter(level);

		/// <summary>Writes a formatted line to the log with the specified
		/// severity level!</summary>
		/// <param name="level">Severity level of this log message.</param>
		/// <param name="text">Formatted text with color tags! See the Log
		/// class docs for guidance on color tags.</param>
		public static void Write(LogLevel level, string text)
			=> NativeAPI.log_write(level, text);

		/// <summary>Writes a formatted line to the log using a
		/// LogLevel.Diagnostic severity level!</summary>
		/// <param name="text">Formatted text with color tags! See the Log
		/// class docs for guidance on color tags.</param>
		public static void Diag(string text)
			=> NativeAPI.log_write(LogLevel.Diagnostic, text);

		/// <summary>Writes a formatted line to the log using a LogLevel.Info
		/// severity level!</summary>
		/// <param name="text">Formatted text with color tags! See the Log
		/// class docs for guidance on color tags.</param>
		public static void Info(string text)
			=> NativeAPI.log_write(LogLevel.Info, text);

		/// <summary>Writes a formatted line to the log using a LogLevel.Warn
		/// severity level!</summary>
		/// <param name="text">Formatted text with color tags! See the Log
		/// class docs for guidance on color tags.</param>
		public static void Warn(string text)
			=> NativeAPI.log_write(LogLevel.Warning, text);

		/// <summary>Writes a formatted line to the log using a
		/// LogLevel.Error severity level!</summary>
		/// <param name="text">Formatted text with color tags! See the Log
		/// class docs for guidance on color tags.</param>
		public static void Err(string text)
			=> NativeAPI.log_write(LogLevel.Error, text);


		/// <summary>Allows you to listen in on log events! Any callback
		/// subscribed here will be called when something is logged. This
		/// does honor the Log.Filter, so filtered logs will not be received
		/// here. This method can safely be called before SK initialization.
		/// </summary>
		/// <param name="onLog">The function to call when a log event occurs.
		/// </param>
		public static void Subscribe(LogCallback onLog) 
		{
			NativeLib.Load();
			_Subscribe(onLog);
		}
		private static void _Subscribe(LogCallback onLog)
		{
			// Separate function because the native call will make C# attempt
			// to load the library as soon as it enters this method.
			callbacks.Add(onLog, (IntPtr context, LogLevel level, string text) => onLog(level, text)); // This prevents the callback from getting GCed
			NativeAPI.log_subscribe(callbacks[onLog], IntPtr.Zero);
		}

		/// <summary>If you subscribed to the log callback, you can
		/// unsubscribe that callback here!
		/// This method can safely be called before initialization.</summary>
		/// <param name="onLog">The subscribed callback to remove.</param>
		public static void Unsubscribe(LogCallback onLog)
		{
			NativeLib.Load();
			_Unsubscribe(onLog);
		}
		private static void _Unsubscribe(LogCallback onLog)
		{
			// Separate function because the native call will make C# attempt
			// to load the library as soon as it enters this method.
			NativeAPI.log_unsubscribe(callbacks[onLog], IntPtr.Zero);
			callbacks.Remove(onLog);
		}
	}
}
