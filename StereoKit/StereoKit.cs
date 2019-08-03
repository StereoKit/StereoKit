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
        static extern int sk_init(string app_name, Runtime runtime);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void sk_shutdown();
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern int sk_step([MarshalAs(UnmanagedType.FunctionPtr)]Action app_update);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern float sk_timef();
        #endregion

        public static bool    IsInitialized { get; private set; }
        public static Runtime ActiveRuntime { get; private set; }
        public static float   Time          { get { return sk_timef(); } }

        public static bool Initialize(string name, Runtime runtimePreference, bool fallback = true)
        {
            // Selects appropriate DLL for the platform, must be called first, and before
            // any StereoKit native calls
            NativeLib.LoadDll();

            ActiveRuntime = runtimePreference;
            IsInitialized = InitializeCall(name, ActiveRuntime);

            // Try falling back to flatscreen, if we didn't just try it
            if (!IsInitialized && fallback && runtimePreference != Runtime.Flatscreen)
            {
                Log.Write(LogLevel.Warning, "Couldn't create StereoKit in {0} mode, falling back to Flatscreen", runtimePreference.ToString());
                ActiveRuntime = Runtime.Flatscreen;
                IsInitialized = InitializeCall(name, ActiveRuntime);
            }

            if (!IsInitialized)
                Log.Write(LogLevel.Error, "Couldn't initialize StereoKit!");

            return IsInitialized;
        }
        private static bool InitializeCall(string name, Runtime runtime)
        {
            // DllImport finds the function at the beginning of the function call,
            // so this needs to be in a separate function from NativeLib.LoadDll
            return sk_init(name, runtime) > 0;
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
