using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;

namespace StereoKit
{
    public enum Runtime
    {
        Flatscreen = 0,
        MixedReality = 1
    }
    public class StereoKitApp
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

        public StereoKitApp()
        {
            NativeLib.LoadDll();
        }
        ~StereoKitApp()
        {
            GC.Collect(GC.MaxGeneration, GCCollectionMode.Forced);
            sk_shutdown();
            NativeLib.UnloadDLL();
        }

        public bool Initialize(string name, Runtime runtime)
        {
            return sk_init(name, runtime) > 0;
        }

        public bool Step(Action onStep)
        {
            return sk_step(onStep) > 0;
        }

        public static float Time { get { return sk_timef(); } }
    }
}
