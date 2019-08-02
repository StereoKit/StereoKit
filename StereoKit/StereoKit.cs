using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

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
        [DllImport(Util.DllName, CharSet = CharSet.Ansi)]
        static extern bool sk_init(string app_name, Runtime runtime);
        [DllImport(Util.DllName)]
        static extern void sk_shutdown();
        [DllImport(Util.DllName)]
        static extern bool sk_step([MarshalAs(UnmanagedType.FunctionPtr)]Action app_update);
        [DllImport(Util.DllName)]
        static extern float sk_timef();
        #endregion

        public StereoKitApp()
        {
        }
        ~StereoKitApp()
        {
            GC.Collect(GC.MaxGeneration, GCCollectionMode.Forced, true, false);
            sk_shutdown();
        }

        public bool Initialize(string name, Runtime runtime)
        {
            return sk_init(name, runtime);
        }

        public bool Step(Action onStep)
        {
            return sk_step(onStep);
        }

        public static float Time { get { return sk_timef(); } }
    }
}
