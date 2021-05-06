using StereoKit.Framework;
using System.Collections.Generic;
using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>This class contains functions for running the StereoKit 
	/// library!</summary>
	public static class SK
	{
		private static SystemInfo   _system;
		private static Steppers     _steppers;
		private static List<Action> _mainThreadInvoke = new List<Action>();

		/// <summary>This is a copy of the settings that StereoKit was
		/// initialized with, so you can refer back to them a little easier.
		/// These are read only, and keep in mind that some settings are 
		/// only requests! Check SK.System and other properties for the 
		/// current state of StereoKit.</summary>
		public static SKSettings Settings { get; private set; }
		/// <summary>Has StereoKit been successfully initialized already? If
		/// initialization was attempted and failed, this value will be 
		/// false.</summary>
		public static bool IsInitialized { get; private set; }
		/// <summary>Since we can fallback to a different DisplayMode, this 
		/// lets you check to see which Runtime was successfully initialized.
		/// </summary>
		public static DisplayMode ActiveDisplayMode => NativeAPI.sk_active_display_mode();
		/// <summary>This structure contains information about the current 
		/// system and its capabilites. There's a lot of different MR devices,
		/// so it's nice to have code for systems with particular 
		/// characteristics!</summary>
		public static SystemInfo System => _system;

		/// <summary>Human-readable version name embedded in the StereoKitC 
		/// DLL.</summary>
		public static string VersionName => Marshal.PtrToStringAnsi( NativeAPI.sk_version_name() );
		/// <summary>An integer version Id! This is defined using a hex value
		/// with this format: `0xMMMMiiiiPPPPrrrr` in order of 
		/// Major.mInor.Patch.pre-Release</summary>
		public static ulong VersionId => NativeAPI.sk_version_id();

		/// <summary>Initializes StereoKit window, default resources, systems,
		/// etc.</summary>
		/// <param name="settings">The configuration settings for StereoKit.
		/// This defines how StereoKit starts up and behaves, so it contains
		/// things like app name, assets folder, display mode, etc. The 
		/// default settings are meant to be good for most, but you may want
		/// to modify at least a few of these eventually!</param>
		/// <returns>Returns true if all systems are successfully 
		/// initialized!</returns>
		public static bool Initialize(SKSettings settings)
		{
			if (!NativeLib.LoadLib()) {
				global::System.Diagnostics.Debug.WriteLine("[SK error] Failed to load StereoKitC!");
				Console.WriteLine                         ("[SK error] Failed to load StereoKitC!");
				return false;
			}

			IsInitialized = InitializeCall(settings);
			return IsInitialized;
		}

		public static void SetWindow(IntPtr window)
			=> NativeAPI.sk_set_window_xam(window);

		private static bool InitializeCall(SKSettings settings)
		{
			// Prep console for colored debug logs
			Log.SetupConsole();

			// DllImport finds the function at the beginning of the function 
			// call, so this needs to be in a separate function from 
			// NativeLib.LoadDll
			bool result = NativeAPI.sk_init(settings) > 0;
			Settings = settings;

			// Get system information
			if (result) { 
				_system = NativeAPI.sk_system_info();
				Default.Initialize();

				_steppers = new Steppers();
				result = _steppers.Initialize();
			}

			return result;
		}

		/// <summary>Shuts down all StereoKit initialized systems. Release
		/// your own StereoKit created assets before calling this.</summary>
		public static void Shutdown()
		{
			if (IsInitialized)
			{
				_steppers.Shutdown();
				Default.Shutdown();
				NativeAPI.sk_shutdown();
			}
		}

		/// <summary>Lets StereoKit know it should quit! It'll finish the 
		/// current frame, and after that Step will return that it wants to 
		/// exit.</summary>
		public static void Quit()
		{
			NativeAPI.sk_quit();
		}

		/// <summary> Steps all StereoKit systems, and inserts user code via
		/// callback between the appropriate system updates. </summary>
		/// <param name="onStep">A callback where you put your application 
		/// code! This gets called between StereoKit systems, after frame 
		/// setup, but before render.</param>
		/// <returns>If an exit message is received from the platform, this 
		/// function will return false.</returns>
		public static bool Step(Action onStep = null)
		{
			return NativeAPI.sk_step(() => {
				_steppers.Step();
				onStep?.Invoke();

				for (int i = 0; i < _mainThreadInvoke.Count; i++) {
					_mainThreadInvoke[i].Invoke();
				}
				_mainThreadInvoke.Clear();
			});
		}

		public static T AddStepper<T>(T stepper) where T:IStepper => _steppers.Add(stepper);
		public static T AddStepper<T>() where T:IStepper => _steppers.Add<T>();
		public static void RemoveStepper(IStepper stepper) => _steppers.Remove(stepper);
		public static void RemoveStepper<T>() where T:IStepper => _steppers.Remove<T>();

		public static void ExecuteOnMain(Action action) => _mainThreadInvoke.Add(action);
	}
}
