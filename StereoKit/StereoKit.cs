using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
    public enum Runtime
    {
        Flatscreen = 0,
        MixedReality = 1
    }
    public static class StereoKitApp
    {
        #region Imports
        [DllImport(NativeLib.DllName, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        static extern int sk_init(string app_name, Runtime preferredRuntime, int fallback);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void sk_shutdown();
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern int sk_step([MarshalAs(UnmanagedType.FunctionPtr)]Action app_update);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern float sk_timef();
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern Runtime sk_active_runtime();
        #endregion

        public static bool    IsInitialized { get; private set; }
        public static Runtime ActiveRuntime { get { return sk_active_runtime(); } }
        public static float   Time          { get { return sk_timef(); } }

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
            return sk_init(name, runtime, fallback?1:0) > 0;
        }

        public static void Shutdown()
        {
            if (IsInitialized)
            {
                sk_shutdown();
                NativeLib.UnloadDLL();
            }
        }

        public static bool Step(Action onStep)
        {
            return sk_step(onStep) > 0;
        }
    }
}
