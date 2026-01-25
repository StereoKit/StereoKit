using System;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
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

			// Android using Xamarin or Maui both use Mono.Android
			if (IsAssemblyLoaded("Mono.Android"))
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
				// If we're on the right thread, this will just work. But
				// there's a good chance we're not, and that's okay.
				loadLibrary.Invoke(null, new object[] { "StereoKitC" });
				return true;
			} catch {
				// We can't invoke on the main thread without the Activity!

				Type activity = Type.GetType("Android.App.Activity, Mono.Android");
				if (activity == null) return false;

				if (SK.AndroidActivity == null)
				{
					// If we have Xamarin.Essentials or Maui.Essentials, we can
					// grab the Activity from their Platform.CurrentActivity
					// property.
					Type platform = null;
					if      (IsAssemblyLoaded("Xamarin.Essentials"       )) platform = Type.GetType("Xamarin.Essentials.Platform, Xamarin.Essentials");
					else if (IsAssemblyLoaded("Microsoft.Maui.Essentials")) platform = Type.GetType("Microsoft.Maui.ApplicationModel.Platform, Microsoft.Maui.Essentials");
					PropertyInfo currentActivity = platform?.GetProperty("CurrentActivity", BindingFlags.Static | BindingFlags.Public);
					if (currentActivity != null)
						SK.AndroidActivity = currentActivity.GetValue(null);
				}
				if (SK.AndroidActivity == null)
				{
					Debug  .WriteLine("[SK error] SK.AndroidActivity must be set before this point!");
					Console.WriteLine("[SK error] SK.AndroidActivity must be set before this point!");
					return false;
				}

				// We're likely not on the main/UI thread, so lets get a way to
				// insert our code onto the main/UI thread.
				MethodInfo runOnUiThread = activity.GetMethod("RunOnUiThread", new Type[] { typeof(Action) });
				if (runOnUiThread == null) { Debug.WriteLine("[SK error] Could not find Android.App.Activity.RunOnUiThread"); return false; }

				// Ask the UI thread to load the libraries, then sleep this
				// thread until it has succeeded or failed.
				int loaded = 0;
				Action loadLibs = () => {
					try   { loadLibrary.Invoke(null, new object[] { "StereoKitC" }); loaded = 1; }
					catch (Exception e) { loaded = -1; Debug.WriteLine("[SK error] Exception loading StereoKitC:\n"+e.ToString()); }
				};
				try   { runOnUiThread.Invoke(SK.AndroidActivity, new object[] { loadLibs }); }
				catch { loaded = -1; }

				while (loaded == 0)
					Thread.Sleep(1);
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
			bool result = false;

			try { result = LoadUnix2(arch); }
			catch (DllNotFoundException) { }
			if (result == false)
			{
				try { result = LoadUnix1(arch); }
				catch (DllNotFoundException) { }
			}

			return result;
		}

		[DllImport("dl", CharSet = CharSet.Ansi)]
		static extern IntPtr dlopen(string fileName, int flags);
		static bool LoadUnix1(string arch)
		{
			const int RTLD_NOW = 2;
			bool isMac = RuntimeInformation.IsOSPlatform(OSPlatform.OSX);
			string os = isMac ? "osx" : "linux";
			string ext = isMac ? "dylib" : "so";

			// On macOS, load MoltenVK first (required for Vulkan support)
			if (isMac)
			{
				dlopen($"./runtimes/{os}-{arch}/native/libMoltenVK.dylib", RTLD_NOW);
				dlopen($"{AppDomain.CurrentDomain.BaseDirectory}/runtimes/{os}-{arch}/native/libMoltenVK.dylib", RTLD_NOW);
				dlopen("libMoltenVK.dylib", RTLD_NOW);
			}

			if (dlopen($"./runtimes/{os}-{arch}/native/libStereoKitC.{ext}", RTLD_NOW)                                       != IntPtr.Zero) return true;
			if (dlopen($"{AppDomain.CurrentDomain.BaseDirectory}/runtimes/{os}-{arch}/native/libStereoKitC.{ext}", RTLD_NOW) != IntPtr.Zero) return true;
			if (dlopen($"libStereoKitC.{ext}", RTLD_NOW)                                                                      != IntPtr.Zero) return true;
			return false;
		}

		// libdl.so.2 is apparently preferred when present?
		[DllImport("libdl.so.2", CharSet = CharSet.Ansi, EntryPoint = "dlopen")]
		static extern IntPtr dlopen2(string fileName, int flags);
		static bool LoadUnix2(string arch)
		{
			const int RTLD_NOW = 2;
			bool isMac = RuntimeInformation.IsOSPlatform(OSPlatform.OSX);
			string os = isMac ? "osx" : "linux";
			string ext = isMac ? "dylib" : "so";

			// On macOS, load MoltenVK first (required for Vulkan support)
			if (isMac)
			{
				dlopen2($"./runtimes/{os}-{arch}/native/libMoltenVK.dylib", RTLD_NOW);
				dlopen2($"{AppDomain.CurrentDomain.BaseDirectory}/runtimes/{os}-{arch}/native/libMoltenVK.dylib", RTLD_NOW);
				dlopen2("libMoltenVK.dylib", RTLD_NOW);
			}

			if (dlopen2($"./runtimes/{os}-{arch}/native/libStereoKitC.{ext}", RTLD_NOW)                                       != IntPtr.Zero) return true;
			if (dlopen2($"{AppDomain.CurrentDomain.BaseDirectory}/runtimes/{os}-{arch}/native/libStereoKitC.{ext}", RTLD_NOW) != IntPtr.Zero) return true;
			if (dlopen2($"libStereoKitC.{ext}", RTLD_NOW)                                                                      != IntPtr.Zero) return true;
			return false;
		}

		static bool IsAssemblyLoaded(string assembly)
			=> AppDomain.CurrentDomain
				.GetAssemblies()
				.Any(a => a.GetName().Name == assembly);
	}
}
