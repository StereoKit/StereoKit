using System;

namespace StereoKit
{
    /// <summary>
    /// This class contains functions for running the StereoKit library!
    /// </summary>
    public static class StereoKitApp
    {
        public static Settings settings;
        public static bool     IsInitialized { get; private set; }
        public static Runtime  ActiveRuntime { get { return NativeAPI.sk_active_runtime(); } }

        /// <summary>Initializes StereoKit window, default resources, systems, etc. Set settings before calling this function, if defaults need changed!</summary>
        /// <param name="name">Name of the application, this shows up an the top of the Win32 window, and is submitted to OpenXR. OpenXR caps this at 128 characters.</param>
        /// <param name="runtimePreference">Which runtime should we try to load?</param>
        /// <param name="fallback">If the preferred runtime fails, should we fall back to flatscreen?</param>
        /// <returns>Returns true if all systems are successfully initialized!</returns>
        public static bool Initialize(string name, Runtime runtimePreference, bool fallback = true)
        {
            IsInitialized = InitializeCall(name, runtimePreference, fallback);
            return IsInitialized;
        }
        private static bool InitializeCall(string name, Runtime runtime, bool fallback)
        {
            // Prep console for colored debug logs
            Log.SetupConsole();

            // DllImport finds the function at the beginning of the function call,
            // so this needs to be in a separate function from NativeLib.LoadDll
            NativeAPI.sk_set_settings(settings);
            return NativeAPI.sk_init(name, runtime, fallback?1:0) > 0;
        }

        /// <summary>
        /// Shuts down all StereoKit initialized systems. Release your own StereoKit created assets before calling this.
        /// </summary>
        public static void Shutdown()
        {
            if (IsInitialized)
            {
                NativeAPI.sk_shutdown();
            }
        }

        /// <summary>
        /// Steps all StereoKit systems, and inserts user code via callback between the appropriate system updates.
        /// </summary>
        /// <param name="onStep">A callback where you put your application code! This gets called between StereoKit systems, after frame setup, but before render.</param>
        /// <returns>If an exit message is received from the platform, this function will return false.</returns>
        public static bool Step(Action onStep)
        {
            return NativeAPI.sk_step(onStep) > 0;
        }
    }
}
