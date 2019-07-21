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
        [DllImport("StereoKitC.dll", CharSet = CharSet.Ansi)]
        static extern bool sk_init(string app_name, Runtime runtime);
        [DllImport("StereoKitC.dll")]
        static extern void sk_shutdown();
        [DllImport("StereoKitC.dll")]
        static extern bool sk_step([MarshalAs(UnmanagedType.FunctionPtr)]Action app_update);
        #endregion

        public StereoKitApp(string name, Runtime runtime)
        {
            sk_init(name, runtime);
        }
        ~StereoKitApp()
        {
            sk_shutdown();
        }

        public bool Step(Action onStep)
        {
            return sk_step(onStep);
        }
    }
}
