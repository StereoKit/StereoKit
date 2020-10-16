using StereoKit.Framework;
using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>This class contains functions for running the StereoKit library!</summary>
	public static class StereoKitApp
	{
		private static SystemInfo _system;
		private static Steppers   _steppers;

		/// <summary>Settings for more detailed initialization of StereoKit! Set these before calling Initialize,
		/// otherwise they'll be ignored.</summary>
		public static Settings   settings;
		/// <summary>Has StereoKit been successfully initialized already? If initialization was attempted and 
		/// failed, this value will be false.</summary>
		public static bool       IsInitialized { get; private set; }
		/// <summary>Since we can fallback to a different Runtime, this lets you check to see which Runtime
		/// was successfully initialized.</summary>
		public static Runtime    ActiveRuntime => NativeAPI.sk_active_runtime();
		/// <summary>This structure contains information about the current system and its capabilites. There's a 
		/// lot of different MR devices, so it's nice to have code for systems with particular characteristics!</summary>
		public static SystemInfo System => _system;

		/// <summary>Human-readable version name embedded in the StereoKitC DLL.</summary>
		public static string VersionName => Marshal.PtrToStringAnsi( NativeAPI.sk_version_name() );
		/// <summary>An integer version Id! This is defined using a hex value with this format: </summary>
		/// `0xMMMMiiiiPPPPrrrr` in order of Major.mInor.Patch.pre-Release
		public static ulong VersionId => NativeAPI.sk_version_id();

		/// <summary>Initializes StereoKit window, default resources, systems, etc. Set settings before calling 
		/// this function, if defaults need changed!</summary>
		/// <param name="name">Name of the application, this shows up an the top of the Win32 window, and is 
		/// submitted to OpenXR. OpenXR caps this at 128 characters.</param>
		/// <param name="runtimePreference">Which runtime should we try to load?</param>
		/// <param name="fallback">If the preferred runtime fails, should we fall back to flatscreen?</param>
		/// <returns>Returns true if all systems are successfully initialized!</returns>
		public static bool Initialize(string name, Runtime runtimePreference = Runtime.MixedReality, bool fallback = true)
		{
			IsInitialized = InitializeCall(name, runtimePreference, fallback);
			return IsInitialized;
		}
		public static bool InitializeAndroid(string name, IntPtr windowHandle, Runtime runtimePreference = Runtime.MixedReality, bool fallback = true)
		{
			IsInitialized = InitializeCallAndroid(name, windowHandle, runtimePreference, fallback);
			return IsInitialized;
		}

		private static bool InitializeCall(string name, Runtime runtime, bool fallback)
		{
			// Prep console for colored debug logs
			Log.SetupConsole();

			// DllImport finds the function at the beginning of the function call,
			// so this needs to be in a separate function from NativeLib.LoadDll
			NativeAPI.sk_set_settings(settings);
			bool result = NativeAPI.sk_init(name, runtime, fallback ? 1 : 0) > 0;

			// Get system information
			if (result) { 
				_system = NativeAPI.sk_system_info();
				Default.Initialize();

				_steppers = new Steppers();
				result = _steppers.Initialize();
			}

			return result;
		}

		private static bool InitializeCallAndroid(string name, IntPtr windowHandle, Runtime runtime, bool fallback)
		{
			// Prep console for colored debug logs
			Log.SetupConsole();

			// DllImport finds the function at the beginning of the function call,
			// so this needs to be in a separate function from NativeLib.LoadDll
			NativeAPI.sk_set_settings(settings);
			bool result = NativeAPI.sk_init_from(windowHandle, name, runtime, fallback ? 1 : 0) > 0;

			// Get system information
			if (result) { 
				_system = NativeAPI.sk_system_info();
				Default.Initialize();

				_steppers = new Steppers();
				result = _steppers.Initialize();
			}

			return result;
		}

		/// <summary>Shuts down all StereoKit initialized systems. Release your own StereoKit created assets before calling this.</summary>
		public static void Shutdown()
		{
			if (IsInitialized)
			{
				_steppers.Shutdown();
				Default.Shutdown();
				NativeAPI.sk_shutdown();
			}
		}

		/// <summary>Lets StereoKit know it should quit! It'll finish the current frame, and after that Step
		/// will return that it wants to exit.</summary>
		public static void Quit()
		{
			NativeAPI.sk_quit();
		}

		/// <summary> Steps all StereoKit systems, and inserts user code via callback between the appropriate system updates. </summary>
		/// <param name="onStep">A callback where you put your application code! This gets called between StereoKit systems, after frame setup, but before render.</param>
		/// <returns>If an exit message is received from the platform, this function will return false.</returns>
		public static bool Step(Action onStep = null)
		{
			return NativeAPI.sk_step(() => {
				_steppers.Step();
				onStep?.Invoke();
			});
		}

		public static T AddStepper<T>(T stepper) where T:IStepper => _steppers.Add(stepper);
		public static T AddStepper<T>() where T:IStepper => _steppers.Add<T>();
		public static void RemoveStepper(IStepper stepper) => _steppers.Remove(stepper);
		public static void RemoveStepper<T>() where T:IStepper => _steppers.Remove<T>();
	}
}
