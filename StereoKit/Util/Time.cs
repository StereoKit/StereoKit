using System;
using System.Collections.Generic;
using System.Text;

namespace StereoKit
{
    class Time
    {
        public static double Total    { get { return NativeAPI.time_get     (); } }
        public static float  Totalf   { get { return NativeAPI.time_getf    (); } }
        public static double Elapsed  { get { return NativeAPI.time_elapsed (); } }
        public static float  Elapsedf { get { return NativeAPI.time_elapsedf(); } }
    }
}
