using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>A vector with 2 components: x and y. This can represent a 
	/// point in 2D space, a directional vector, or any other sort of value 
	/// with 2 dimensions to it!</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct Vec2
	{
		/// <summary>Vector components.</summary>
		public float x, y;

		/// <summary>Magnitude is the length of the vector! Or the distance 
		/// from the origin to this point. Uses Math.Sqrt, so it's not dirt 
		/// cheap or anything.</summary>
		public float Magnitude => SKMath.Sqrt(x*x + y*y);
        
		/// <summary>This is the squared magnitude of the vector! It skips 
		/// the Sqrt call, and just gives you the squared version for speedy 
		/// calculations that can work with it squared.</summary>
		public float MagnitudeSq => x*x + y*y;

		public Vec3 XY0 { get => new Vec3(x, y, 0); }
		public Vec3 X0Y { get => new Vec3(x, 0, y); }

		/// <summary>A basic constructor, just copies the values in!</summary>
		/// <param name="x">X component of the vector.</param>
		/// <param name="y">Y component of the vector.</param>
		public Vec2(float x, float y)
		{ 
			this.x = x;
			this.y = y;
		}

		/// <summary>Creates a normalized vector (vector with a length of 1) 
		/// from the current vector. Will not work properly if the vector has 
		/// a length of zero.</summary>
		/// <returns>The normalized (length of 1) vector!</returns>
		public Vec2 Normalized()
		{
			float mag = SKMath.Sqrt(x*x+y*y);
			return new Vec2(x/mag, y/mag);
		}

		/// <summary>Turns this vector into a normalized vector (vector with 
		/// a length of 1) from the current vector. Will not work properly if
		/// the vector has a length of zero.</summary>
		/// <returns>The normalized (length of 1) vector!</returns>
		public void Normalize()
		{
			float mag = SKMath.Sqrt(x*x+y*y);
			x /= mag;
			y /= mag;
		}

		/// <summary>Returns the counter-clockwise degrees from [1,0]. 
		/// Resulting value is between 0 and 360. Vector does not need to be
		/// normalized.</summary>
		/// <returns>Counter-clockwise angle of the vector in degrees from
		/// [1,0], as a value between 0 and 360.</returns>
		public float Angle()
		{
			float result = (float)Math.Atan2(y, x) * Units.rad2deg;
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
		public static float AngleBetween(Vec2 a, Vec2 b)
			=> (float)Math.Atan2(a.x*b.y - a.y*b.x, a.x*b.x + a.y*b.y) * Units.rad2deg;

		/// <summary>Calculates the distance between two points in space! Make sure they're in the
		/// same coordinate space! Uses a Sqrt, so it's not blazing fast, prefer DistanceSq when possible.</summary>
		/// <param name="a">The first point.</param>
		/// <param name="b">And the second point!</param>
		/// <returns>Distance between the two points.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Distance(in Vec2 a, in Vec2 b)
			{Vec2 d = b-a; return SKMath.Sqrt(d.x*d.x + d.y*d.y);}

		/// <summary>Calculates the distance between two points in space, but leaves them squared! Make 
		/// sure they're in the same coordinate space! This is a fast function :)</summary>
		/// <param name="a">The first point.</param>
		/// <param name="b">And the second point!</param>
		/// <returns>Distance between the two points, but squared!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float DistanceSq(in Vec2 a, in Vec2 b)
			{Vec2 d = b - a; return d.x*d.x + d.y*d.y;}

		/// <summary>The dot product is an extremely useful operation! One major use is to determine
		/// how similar two vectors are. If the vectors are Unit vectors (magnitude/length of 1), then 
		/// the result will be 1 if the vectors are the same, -1 if they're opposite, and a gradient 
		/// in-between with 0 being perpendicular. See [Freya Holmer's excellent visualization](https://twitter.com/FreyaHolmer/status/1200807790580768768)
		/// of this concept</summary>
		/// <param name="a">First vector.</param>
		/// <param name="b">Second vector.</param>
		/// <returns>The dot product!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Dot(in Vec2 a, in Vec2 b)
			=> a.x*b.x + a.y*b.y;

		public override string ToString()
		{
			return string.Format("[{0:0.00}, {1:0.00}]", x, y);
		}

		/// <summary>A Vec2 with all components at zero, same as new Vec2(0,0).</summary>
		public static readonly Vec2 Zero = new Vec2(0,0);
		/// <summary>A Vec2 with all components at one, same as new Vec2(1,1).</summary>
		public static readonly Vec2 One  = new Vec2(1,1);
		public static Vec2 operator +(Vec2 a, Vec2 b)  { return new Vec2(a.x + b.x, a.y + b.y); }
		public static Vec2 operator -(Vec2 a, Vec2 b)  { return new Vec2(a.x - b.x, a.y - b.y); }
		public static Vec2 operator -(Vec2 a)          { return new Vec2(-a.x, -a.y); }
		public static Vec2 operator *(Vec2 a, float b) { return new Vec2(a.x * b, a.y * b); }
		public static Vec2 operator /(Vec2 a, float b) { return new Vec2(a.x / b, a.y / b); }
	}
}
