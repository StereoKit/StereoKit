using System;

namespace StereoKit
{
	/// <summary>This class contains some convenience math functions!
	/// StereoKit typically uses floats instead of doubles, so you won't need 
	/// to cast to and from using these methods.</summary>
	public static class SKMath
	{
		/// <summary>The mathematical constant, Pi!</summary>
		public const float Pi  = 3.14159265359f;
		/// <summary>Tau is 2*Pi, there are excellent arguments that Tau can
		/// make certain formulas more readable!</summary>
		public const float Tau = 6.28318530718f;

		/// <summary>Same as Math.Cos</summary>
		/// <returns>Same as Math.Cos</returns>
		public static float Cos(float f) => (float)Math.Cos(f);
		/// <summary>Same as Math.Sin</summary>
		/// <returns>Same as Math.Sin</returns>
		public static float Sin(float f) => (float)Math.Sin(f);
		/// <summary>Same as Math.Sqrt</summary>
		/// <returns>Same as Math.Sqrt</returns>
		public static float Sqrt(float f) => (float)Math.Sqrt(f);
		/// <summary>Same as Math.Exp</summary>
		/// <returns>Same as Math.Exp</returns>
		public static float Exp(float f) => (float)Math.Exp(f);

		/// <summary>Blends (Linear Interpolation) between two scalars, based
		/// on a 'blend' value, where 0 is a, and 1 is b. Doesn't clamp
		/// percent for you.</summary>
		/// <param name="a">First item in the blend, or '0.0' blend.</param>
		/// <param name="b">Second item in the blend, or '1.0' blend.</param>
		/// <param name="t">A blend value between 0 and 1. Can be outside
		/// this range, it'll just interpolate outside of the a, b range.
		/// </param>
		/// <returns>An unclamped blend of a and b.</returns>
		public static float Lerp(float a, float b, float t) => a + (b-a)*t;

		/// <summary>Calculates the minimum angle 'distance' between two 
		/// angles. This covers wraparound cases like: the minimum distance 
		/// between 10 and 350 is 20.</summary>
		/// <param name="a">First angle, in degrees.</param>
		/// <param name="b">Second angle, in degrees.</param>
		/// <returns>Degrees 0-180, the minimum angle between a and b.
		/// </returns>
		public static float AngleDist(float a, float b)
		{
			float delta = (b - a + 180) % 360 - 180;
			return Math.Abs(delta < -180 ? delta + 360 : delta);
		}

        /// <summary> Returns value clamped to the inclusive range of min and max.</summary>
		/// <exception cref="ArgumentOutOfRangeException">Throws if min is higher than max</exception>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Clamp(float value, float min, float max)
        {
            // Ported from dotnet 7 MathF.Clamp
            if (min > max)
            {
                throw new ArgumentOutOfRangeException($"Min ({min}) was higher than max ({max}), this is usually a mistake.");
            }

            if (value < min)
                return min;
            else if (value > max)
                return max;
            return value;
        }

		/// <summary>Modulus, works better than '%' for negative values.
		/// </summary>
		/// <param name="x">Numerator</param>
		/// <param name="mod">Denominator</param>
		/// <returns>x modulus mod</returns>
		public static int Mod(int x, int mod)
		{
			int r = x % mod;
			return r<0 ? r+mod : r;
		}
	}
}
