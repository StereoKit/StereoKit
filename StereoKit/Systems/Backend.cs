using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>This class exposes some of StereoKit's backend functionality.
	/// This allows for tighter integration with certain platforms, but also
	/// means your code becomes less portable. Everything in this class should
	/// be guarded by availability checks.</summary>
	public static class Backend
	{
		/// <summary>What technology is being used to drive StereoKit's XR
		/// functionality? OpenXR is the most likely candidate here, but if
		/// you're running the flatscreen Simulator, or running in the web with
		/// WebXR, then this will reflect that.</summary>
		public static BackendXRType XRType => NativeAPI.backend_xr_get_type();

		/// <summary>This class is NOT of general interest, unless you are
		/// trying to add support for some unusual OpenXR extension! StereoKit
		/// should do all the OpenXR work that most people will need. If you
		/// find yourself here anyhow for something you feel StereoKit should
		/// support already, please add a feature request on Github!
		/// 
		/// This class contains handles and methods for working directly with
		/// OpenXR. This may allow you to activate or work with OpenXR
		/// extensions that StereoKit hasn't implemented or exposed yet. Check
		/// that Backend.XRType is OpenXR before using any of this.
		/// 
		/// These properties may best be used with some external OpenXR
		/// binding library, but you may get some limited mileage with the API
		/// as provided here.</summary>
		public static class OpenXR
		{
			/// <summary>Type: XrInstance. StereoKit's instance handle, valid
			/// after SK.Initialize.</summary>
			public static ulong Instance => NativeAPI.backend_openxr_get_instance();

			/// <summary>Type: XrSession. StereoKit's current session handle, 
			/// this will be valid after SK.Initialize, but the session may not
			/// be started quite so early.</summary>
			public static ulong Session => NativeAPI.backend_openxr_get_session();

			/// <summary>Type: XrSpace. StereoKit's primary coordinate space,
			/// valid after SK.Initialize, this will most likely be created
			/// from `XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT` or
			/// `XR_REFERENCE_SPACE_TYPE_LOCAL`.</summary>
			public static ulong Space => NativeAPI.backend_openxr_get_space();

			/// <summary>Type: XrTime. This is the OpenXR time for the current
			/// frame, and is available after SK.Initialize.</summary>
			public static long Time => NativeAPI.backend_openxr_get_time();

			/// <summary>Type: XrTime. This is the OpenXR time of the eye tracker sample associated
			/// with the current value of <see cref="Input.Eyes"/>.</summary>
			public static long EyesSampleTime => NativeAPI.backend_openxr_get_eyes_sample_time();

			/// <summary>This tells if an OpenXR extension has been requested
			/// and successfully loaded by the runtime. This MUST only be
			/// called after SK.Initialize.</summary>
			/// <param name="extensionName">The extension name as listed in the
			/// OpenXR spec. For example: "XR_EXT_hand_tracking".</param>
			/// <returns>If the extension is available to use.</returns>
			public static bool ExtEnabled(string extensionName) => NativeAPI.backend_openxr_ext_enabled(extensionName)>0;

			/// <summary>This is basically `xrGetInstanceProcAddr` from OpenXR,
			/// you can use this to get and call functions from an extension
			/// you've loaded. You can use `Marshal.GetDelegateForFunctionPointer`
			/// to turn the result into a delegate that you can call.</summary>
			/// <param name="functionName"></param>
			/// <returns>A function pointer, or null on failure. You can use 
			/// `Marshal.GetDelegateForFunctionPointer` to turn this into a
			/// delegate that you can call.</returns>
			public static IntPtr GetFunctionPtr(string functionName) => NativeAPI.backend_openxr_get_function(functionName);

			/// <summary>This is basically `xrGetInstanceProcAddr` from OpenXR,
			/// you can use this to get and call functions from an extension
			/// you've loaded. This uses `Marshal.GetDelegateForFunctionPointer`
			/// to turn the result into a delegate that you can call.</summary>
			/// <param name="functionName"></param>
			/// <returns>A delegate, or null on failure.</returns>
			public static TDelegate GetFunction<TDelegate>(string functionName) {
				IntPtr fn = NativeAPI.backend_openxr_get_function(functionName);
				if (fn == null) return default;
				return Marshal.GetDelegateForFunctionPointer<TDelegate>(fn);
			}

			/// <summary>Requests that OpenXR load a particular extension. This
			/// MUST be called before SK.Initialize. Note that it's entirely
			/// possible that your extension will not load on certain runtimes,
			/// so be sure to check ExtEnabled to see if it's available to use.
			/// </summary>
			/// <param name="extensionName">The extension name as listed in the
			/// OpenXR spec. For example: "XR_EXT_hand_tracking".</param>
			public  static void RequestExt (string extensionName) { NativeLib.Load(); _RequestExt(extensionName); }
			private static void _RequestExt(string extensionName) => NativeAPI.backend_openxr_ext_request(extensionName);
		}
	}
}
