using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Threading;

namespace StereoKit
{
	static class NativeLib
	{
		static bool _loaded = false;
		internal static IntPtr androidActivityHandle;

		internal static bool Load()
		{
			if (_loaded)
				return true;

			// Browsers should also have their own strategy for linking
			if (RuntimeInformation.OSDescription == "Browser")
				return true;

			// Xamarin and Maui both have Java.Lang.JavaSystem.LoadLibrary(string)
			if (Type.GetType("Java.Lang.JavaSystem, Mono.Android") != null)
			{
				_loaded = LoadAndroid();
				return _loaded;
			}

			string arch = RuntimeInformation.OSArchitecture == Architecture.Arm64
				? "arm64"
				: "x64";
			_loaded = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
				? LoadWindows(arch)
				: LoadUnix   (arch);
			return _loaded;
		}

		static bool LoadAndroid()
		{
			// Xamarin and Maui both have Java.Lang.JavaSystem.LoadLibrary(string),
			// but this isn't present in netstandard2.0. We're checking for it
			// via reflection here.
			Type javaSystem = Type.GetType("Java.Lang.JavaSystem, Mono.Android");
			if (javaSystem == null) return false;
			MethodInfo loadLibrary = javaSystem.GetMethod("LoadLibrary", BindingFlags.Static | BindingFlags.Public);
			if (loadLibrary == null) return false;

			// JavaSystem.LoadLibrary can only run on the UI thread, so we need
			// to get the Activity, and run a callback there.
			Type platform =
				   Type.GetType("Xamarin.Essentials.Platform, Xamarin.Essentials")
				?? Type.GetType("Microsoft.Maui.ApplicationModel.Platform, Microsoft.Maui.Essentials");
			if (platform == null) return false;
			Type activity = Type.GetType("Android.App.Activity, Mono.Android");
			if (activity == null) return false;
			PropertyInfo currentActivity = platform.GetProperty("CurrentActivity", BindingFlags.Static | BindingFlags.Public);
			if (currentActivity == null) return false;
			MethodInfo runOnUiThread = activity.GetMethod  ("RunOnUiThread", new Type[] { typeof(Action) });
			if (runOnUiThread == null) return false;

			// Grab the activity handle as well, while we're here
			PropertyInfo activityGetHandle = activity.GetProperty("Handle");
			if (activityGetHandle == null) return false;

			object activityObject = currentActivity.GetValue(null);
			androidActivityHandle = (IntPtr)activityGetHandle.GetValue(activityObject);

			// Ask the UI thread to load the libraries, then sleep this thread
			// until it has succeeded or failed.
			int loaded = 0;
			Action loadLibs = () => {
				try
				{
					// Android has no universally supported openxr_loader yet,
					// so on this platform we don't static link it, and instead
					// provide devs a way to ship other loaders.
					loadLibrary.Invoke(null, new object[] { "openxr_loader" });
					loadLibrary.Invoke(null, new object[] { "StereoKitC" });
					loaded = 1;
				}
				catch { loaded = -1; }
			};
			runOnUiThread.Invoke(activityObject, new object[] { loadLibs });

			while (loaded == 0) Thread.Sleep(10);
			return loaded == 1;
		}

		[DllImport("kernel32", CharSet = CharSet.Unicode)]
		static extern IntPtr LoadLibraryW(string fileName);
		static bool LoadWindows(string arch)
		{
			if (LoadLibraryW($"{AppDomain.CurrentDomain.BaseDirectory}/runtimes/win-{arch}/native/StereoKitC.dll") != IntPtr.Zero) return true;
			if (LoadLibraryW("StereoKitC") != IntPtr.Zero) return true;
			return false;
		}

		static bool LoadUnix(string arch)
		{
			try
			{
				return LoadUnix1(arch);
			}
			catch (DllNotFoundException)
			{
				return LoadUnix2(arch);
			}
		}

		[DllImport("dl", CharSet = CharSet.Ansi)]
		static extern IntPtr dlopen(string fileName, int flags);
		static bool LoadUnix1(string arch)
		{
			const int RTLD_NOW = 2;
			if (dlopen($"./runtimes/linux-{arch}/native/libStereoKitC.so", RTLD_NOW) != IntPtr.Zero) return true;
			if (dlopen($"{AppDomain.CurrentDomain.BaseDirectory}/runtimes/linux-{arch}/native/libStereoKitC.so", RTLD_NOW) != IntPtr.Zero) return true;
			if (dlopen("libStereoKitC.so", RTLD_NOW) != IntPtr.Zero) return true;
			return false;
		}

		// A hack to support 
		[DllImport("libdl.so.2", CharSet = CharSet.Ansi, EntryPoint = "dlopen")]
		static extern IntPtr dlopen2(string fileName, int flags);
		static bool LoadUnix2(string arch)
		{
			const int RTLD_NOW = 2;
			if (dlopen2($"./runtimes/linux-{arch}/native/libStereoKitC.so", RTLD_NOW) != IntPtr.Zero) return true;
			if (dlopen2($"{AppDomain.CurrentDomain.BaseDirectory}/runtimes/linux-{arch}/native/libStereoKitC.so", RTLD_NOW) != IntPtr.Zero) return true;
			if (dlopen2("libStereoKitC.so", RTLD_NOW) != IntPtr.Zero) return true;
			return false;
		}
	}
}
