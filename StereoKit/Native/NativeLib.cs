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
			// Due to a long-standing bug, Mono can only load libraries from
			// the main or UI threads, so we need to grab one of those threads
			// and ensure the loading happens there.
			// For reference: https://github.com/xamarin/xamarin-android/issues/7616

			// Grab Java.Lang.JavaSystem.LoadLibrary(string) via reflection.
			Type javaSystem = Type.GetType("Java.Lang.JavaSystem, Mono.Android");
			if (javaSystem == null) return false;
			MethodInfo loadLibrary = javaSystem.GetMethod("LoadLibrary", BindingFlags.Static | BindingFlags.Public);
			if (loadLibrary == null) return false;

			try {
				// If we're on the right thread, this will just work.
				loadLibrary.Invoke(null, new object[] { "StereoKitC" });
				return true;
			} catch {
				// We can't invoke on the main thread without the Activity!
				if (SK.AndroidActivity == null)
				{
					System.Diagnostics.Debug.WriteLine("[SK error] SK.AndroidActivity must be set before this point!");
					Console.WriteLine                 ("[SK error] SK.AndroidActivity must be set before this point!");
					return false;
				}

				// We're likely not on the main/UI thread, so lets get a way to
				// insert our code onto the main/UI thread.
				Type activity = Type.GetType("Android.App.Activity, Mono.Android");
				if (activity == null) return false;
				MethodInfo runOnUiThread = activity.GetMethod  ("RunOnUiThread", new Type[] { typeof(Action) });
				if (runOnUiThread == null) return false;

				// Ask the UI thread to load the libraries, then sleep this
				// thread until it has succeeded or failed.
				int loaded = 0;
				Action loadLibs = () => {
					try { loadLibrary.Invoke(null, new object[] { "StereoKitC" }); loaded = 1; }
					catch { loaded = -1; }
				};
				try { runOnUiThread.Invoke(SK.AndroidActivity, new object[] { loadLibs }); }
				catch { loaded = -1; }

				while (loaded == 0)
					Thread.Sleep(10);
				return loaded == 1;
			}
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
			try                          { return LoadUnix1(arch); }
			catch (DllNotFoundException) { return LoadUnix2(arch); }
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
