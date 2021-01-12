using System;
using System.Numerics;
using System.Runtime.CompilerServices;

namespace StereoKit
{
	/// <summary>A vector with 2 components: x and y. This can represent a 
	/// point in 2D space, a directional vector, or any other sort of value 
	/// with 2 dimensions to it!</summary>
	public static class Vec2
	{
		/// <summary>A Vec2 with all components at zero, same as new Vec2(0,0).</summary>
		public static readonly Vector2 Zero = new Vector2(0, 0);
		/// <summary>A Vec2 with all components at one, same as new Vec2(1,1).</summary>
		public static readonly Vector2 One = new Vector2(1, 1);

		/// <summary>Returns the counter-clockwise degrees from [1,0]. 
		/// Resulting value is between 0 and 360. Vector does not need to be
		/// normalized.</summary>
		/// <returns>Counter-clockwise angle of the vector in degrees from
		/// [1,0], as a value between 0 and 360.</returns>
		public static float Angle(Vector2 v)
		{
			float result = (float)Math.Atan2(v.Y, v.X) * Units.rad2deg;
			if (result < 0) result = 360 + result;
			return result;
		}

		/// <summary>Calculates a signed angle between two vectors! Sign will
		/// be positive if B is counter-clockwise (left) of A, and negative 
		/// if B is clockwise (right) of A. Vectors do not need to be 
		/// normalized.</summary>
		/// <param name="a">The first, initial vector, A. Does not need to be
		/// normalized.</param>
		/// <param name="b">The second vector, B, that we're finding the 
		/// angle to. Does not need to be normalized.</param>
		/// <returns>a signed angle between two vectors! Sign will be 
		/// positive if B is counter-clockwise (left) of A, and negative if B
		/// is clockwise (right) of A.</returns>
		public static float AngleBetween(Vector2 a, Vector2 b)
			=> (float)Math.Atan2(a.X*b.Y - a.Y*b.X, a.X*b.X + a.Y*b.Y) * Units.rad2deg;

		/// <summary>Calculates the distance between two points in space! Make sure they're in the
		/// same coordinate space! Uses a Sqrt, so it's not blazing fast, prefer DistanceSq when possible.</summary>
		/// <param name="a">The first point.</param>
		/// <param name="b">And the second point!</param>
		/// <returns>Distance between the two points.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Distance(in Vector2 a, in Vector2 b) => (b-a).Length();

		/// <summary>Calculates the distance between two points in space, but leaves them squared! Make 
		/// sure they're in the same coordinate space! This is a fast function :)</summary>
		/// <param name="a">The first point.</param>
		/// <param name="b">And the second point!</param>
		/// <returns>Distance between the two points, but squared!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float DistanceSq(in Vector2 a, in Vector2 b) => (b-a).LengthSquared();
	}
}
