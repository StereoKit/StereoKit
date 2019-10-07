using System;

namespace StereoKit
{
    public static class StereoKitApp
    {
        public static Settings settings;
        public static bool     IsInitialized { get; private set; }
        public static Runtime  ActiveRuntime { get { return NativeAPI.sk_active_runtime(); } }
        
        public static bool Initialize(string name, Runtime runtimePreference, bool fallback = true)
        {
            // Selects appropriate DLL for the platform, must be called first, and before
            // any StereoKit native calls
            NativeLib.LoadDll();

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

        public static void Shutdown()
        {
            if (IsInitialized)
            {
                NativeAPI.sk_shutdown();
                NativeLib.UnloadDLL();
            }
        }

        public static bool Step(Action onStep)
        {
            return NativeAPI.sk_step(onStep) > 0;
        }
    }
}
