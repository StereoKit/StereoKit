using System;
using System.Collections.Generic;
using System.Text;

namespace StereoKit
{
    /// <summary>This class contains some convenience math functions! StereoKit
    /// typically uses floats instead of doubles, so you won't need to cast to
    /// and from using these methods.</summary>
    public static class SKMath
    {
        /// <summary>Same as Math.Cos</summary>
        /// <returns>Same as Math.Cos</returns>
        public static float Cos(float f) => (float)Math.Cos(f);
        /// <summary>Same as Math.Sin</summary>
        /// <returns>Same as Math.Sin</returns>
        public static float Sin(float f) => (float)Math.Sin(f);
        /// <summary>Same as Math.Sqrt</summary>
        /// <returns>Same as Math.Sqrt</returns>
        public static float Sqrt(float f) => (float)Math.Sqrt(f);
    }
}
