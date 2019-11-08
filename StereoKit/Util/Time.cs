using System;
using System.Collections.Generic;
using System.Text;

namespace StereoKit
{
    /// <summary>This class contains time information for the current session and frame!</summary>
    class Time
    {
        /// <summary> How much time has elapsed since StereoKit was initialized? 64 bit time precision. </summary>
        public static double Total    { get { return NativeAPI.time_get     (); } }
        /// <summary> How much time has elapsed since StereoKit was initialized? 32 bit time precision. </summary>
        public static float  Totalf   { get { return NativeAPI.time_getf    (); } }
        /// <summary> How much time elapsed during the last frame? 64 bit time precision. </summary>
        public static double Elapsed  { get { return NativeAPI.time_elapsed (); } }
        /// <summary> How much time elapsed during the last frame? 32 bit time precision. </summary>
        public static float  Elapsedf { get { return NativeAPI.time_elapsedf(); } }
    }
}
