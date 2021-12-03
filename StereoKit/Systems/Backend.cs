using System;

namespace StereoKit
{
	public static class Backend
	{
		public static BackendXRType XRType => NativeAPI.backend_xr_get_type();

		public static class OpenXR
		{
			public static ulong Instance => NativeAPI.backend_openxr_get_instance();
			public static ulong Session => NativeAPI.backend_openxr_get_session();
			public static ulong Space => NativeAPI.backend_openxr_get_space();
			public static long Time => NativeAPI.backend_openxr_get_time();
			public static bool ExtEnabled(string extensionName) => NativeAPI.backend_openxr_ext_enabled(extensionName)>0;
			public static IntPtr GetFunction(string functionName) => NativeAPI.backend_openxr_get_function(functionName);

			public  static void RequestExt (string extensionName) { NativeLib.Load(); _RequestExt(extensionName); }
			private static void _RequestExt(string extensionName) => NativeAPI.backend_openxr_ext_request(extensionName);
		}
	}
}
