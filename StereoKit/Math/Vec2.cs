using System;
using System.Numerics;
using System.Runtime.CompilerServices;

namespace StereoKit
{
	/// <summary>A vector with 2 components: x and y. This can represent a 
	/// point in 2D space, a directional vector, or any other sort of value 
	/// with 2 dimensions to it!</summary>
	public struct Vec2
	{
		/// <summary>The internal, wrapped System.Numerics type. This can be
		/// nice to have around so you can pass its fields as 'ref', which
		/// you can't do with properties. You won't often need this, as
		/// implicit conversions to System.Numerics types are also
		/// provided.</summary>
		public Vector2 v;
		/// <summary>X component.</summary>
		public float x { get => v.X; set => v.X=value; }
		/// <summary>Y component.</summary>
		public float y { get => v.Y; set => v.Y=value; }

		/// <summary>A basic constructor, just copies the values in!</summary>
		/// <param name="x">X component of the vector.</param>
		/// <param name="y">Y component of the vector.</param>
		public Vec2(float x, float y) => v = new Vector2(x, y);

		/// <summary>A short hand constructor, just sets all values as the same!</summary>
		/// <param name="xy">X and Y component of the vector.</param>
		public Vec2(float xy) => v = new Vector2(xy, xy);

		/// <summary>Allows implicit conversion from System.Numerics.Vector2
		/// to StereoKit.Vec2.</summary>
		/// <param name="v">Any System.Numerics Vector2.</param>
		/// <returns>A StereoKit compatible vector.</returns>
		public static implicit operator Vec2(Vector2 v) => new Vec2(v.X, v.Y);
		/// <summary>Allows implicit conversion from StereoKit.Vec2 to
		/// System.Numerics.Vector2</summary>
		/// <param name="v">Any StereoKit.Vec2.</param>
		/// <returns>A System.Numerics compatible vector.</returns>
		public static implicit operator Vector2(Vec2 v) => v.v;

		/// <summary>Adds matching components together. Commutative.</summary>
		/// <param name="a">Any vector.</param>
		/// <param name="b">Any vector.</param>
		/// <returns>A new vector from the added components.</returns>
		public static Vec2 operator +(Vec2 a, Vec2 b) => a.v+b.v;
		/// <summary>Subtracts matching components from eachother. Not
		/// commutative.</summary>
		/// <param name="a">Any vector.</param>
		/// <param name="b">Any vector.</param>
		/// <returns>A new vector from the subtracted components.</returns>
		public static Vec2 operator -(Vec2 a, Vec2 b) => a.v-b.v;
		/// <summary>A component-wise vector multiplication, same thing as
		/// a non-uniform scale. NOT a dot product! Commutative.</summary>
		/// <param name="a">Any vector.</param>
		/// <param name="b">Any vector.</param>
		/// <returns>A new vector a scaled by b.</returns>
		public static Vec2 operator *(Vec2 a, Vec2 b) => a.v*b.v;
		/// <summary>A component-wise vector division. Not commutative</summary>
		/// <param name="a">Any vector.</param>
		/// <param name="b">Any vector.</param>
		/// <returns>A new vector a divided by b.</returns>
		public static Vec2 operator /(Vec2 a, Vec2 b) => a.v/b.v;
		/// <summary>Vector negation, returns a vector where each component has
		/// been negated.</summary>
		/// <param name="a">Any vector.</param>
		/// <returns>A vector where each component has been negated.</returns>
		public static Vec2 operator -(Vec2 a) => -a.v;
		/// <summary>A scalar vector multiplication.</summary>
		/// <param name="a">Any vector.</param>
		/// <param name="b">Any scalar.</param>
		/// <returns>A new vector a scaled by b.</returns>
		public static Vec2 operator *(Vec2 a, float b) => a.v*b;
		/// <summary>A scalar vector division.</summary>
		/// <param name="a">Any vector.</param>
		/// <param name="b">Any scalar.</param>
		/// <returns>A new vector a divided by b.</returns>
		public static Vec2 operator /(Vec2 a, float b) => a.v/b;
		/// <summary>A scalar vector multiplication.</summary>
		/// <param name="a">Any scalar.</param>
		/// <param name="b">Any vector.</param>
		/// <returns>A new vector a scaled by b.</returns>
		public static Vec2 operator *(float a, Vec2 b) => b.v*a;

		/// <summary>A Vec2 with all components at zero, this is the same as
		/// `new Vec2(0,0)`.</summary>
		public static readonly Vec2 Zero = new Vec2(0, 0);
		/// <summary>A Vec2 with all components at one, this is the same as
		/// `new Vec2(1,1)`.</summary>
		public static readonly Vec2 One = new Vec2(1, 1);
		/// <summary>A normalized Vector that points down the X axis, this is
		/// the same as `new Vec2(1,0)`.</summary>
		public static readonly Vec2 UnitX = new Vec2(1,0);
		/// <summary>A normalized Vector that points down the Y axis, this is
		/// the same as `new Vec2(0,1)`.</summary>
		public static readonly Vec2 UnitY = new Vec2(0,1);

		/// <summary>A transpose swizzle property, returns (y,x)</summary>
		public Vec2 YX => new Vec2(y, x);
		/// <summary>Promotes this Vec2 to a Vec3, using 0 for the Z axis.
		/// </summary>
		public Vec3 XY0 => new Vec3(x, y, 0);
		/// <summary>Promotes this Vec2 to a Vec3, using 0 for the Y axis.
		/// </summary>
		public Vec3 X0Y => new Vec3(x, 0, y);

		/// <summary>Magnitude is the length of the vector! Or the distance 
		/// from the origin to this point. Uses Math.Sqrt, so it's not dirt 
		/// cheap or anything.</summary>
		public float Magnitude => v.Length();

		/// <summary>This is the length of the vector! Or the distance 
		/// from the origin to this point. Uses Math.Sqrt, so it's not dirt 
		/// cheap or anything.</summary>
		public float Length => v.Length();

		/// <summary>This is the squared magnitude of the vector! It skips 
		/// the Sqrt call, and just gives you the squared version for speedy 
		/// calculations that can work with it squared.</summary>
		public float MagnitudeSq => v.LengthSquared();

		/// <summary>This is the squared length/magnitude of the vector! It
		/// skips the Sqrt call, and just gives you the squared version for
		/// speedy calculations that can work with it squared.</summary>
		public float LengthSq => v.LengthSquared();

		/// <summary>Creates a normalized vector (vector with a length of 1) 
		/// from the current vector. Will not work properly if the vector has 
		/// a length of zero.</summary>
		public Vec2 Normalized => v / v.Length();

		/// <summary>The dot product is an extremely useful operation! One
		/// major use is to determine how similar two vectors are. If the
		/// vectors are Unit vectors (magnitude/length of 1), then the result
		/// will be 1 if the vectors are the same, -1 if they're opposite, 
		/// and a gradient in-between with 0 being perpendicular. See 
		/// [Freya Holmer's excellent visualization](https://twitter.com/FreyaHolmer/status/1200807790580768768)
		/// of this concept</summary>
		/// <param name="a">First vector.</param>
		/// <param name="b">Second vector.</param>
		/// <returns>The dot product!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Dot(Vec2 a, Vec2 b)
			=> Vector2.Dot(a.v, b.v);

		/// <summary>Creates a vector pointing in the direction of the angle,
		/// with a length of 1. Angles are counter-clockwise, and start from
		/// (1,0), so an angle of 90 will be (0,1).</summary>
		/// <param name="degrees">Counter-clockwise angle from (1,0), in
		/// degrees.</param>
		/// <returns>A unit vector (length of 1), pointing towards degrees.
		/// </returns>
		public static Vec2 FromAngle(float degrees)
		{
			float ang = degrees * Units.deg2rad;
			return new Vec2(SKMath.Cos(ang), SKMath.Sin(ang));
		}

		/// <summary>Returns the counter-clockwise degrees from [1,0]. 
		/// Resulting value is between 0 and 360. Vector does not need to be
		/// normalized.</summary>
		/// <returns>Counter-clockwise angle of the vector in degrees from
		/// [1,0], as a value between 0 and 360.</returns>
		public float Angle()
		{
			float result = (float)Math.Atan2(v.Y, v.X) * Units.rad2deg;
			if (result < 0) result = 360 + result;
			return result;
		}

		/// <summary>Turns this vector into a normalized vector (vector with 
		/// a length of 1) from the current vector. Will not work properly if
		/// the vector has a length of zero.</summary>
		/// <returns>The normalized (length of 1) vector!</returns>
		public void Normalize() => v = v / v.Length();

		/// <summary>Checks if a point is within a certain radius of this one.
		/// This is an easily readable shorthand of the squared distance check.
		/// </summary>
		/// <param name="pt">The point to check against.</param>
		/// <param name="radius">The distance to check against.</param>
		/// <returns>True if the points are within radius of each other, false
		/// not.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool InRadius(Vec2 pt, float radius)
			=> (v - pt.v).LengthSquared() < radius * radius;

		/// <summary>Calculates a signed angle between two vectors in degrees!
		/// Sign will be positive if B is counter-clockwise (left) of A, and
		/// negative if B is clockwise (right) of A. Vectors do not need to be 
		/// normalized.
		/// NOTE: Since this will return a positive or negative angle, order of
		/// parameters matters!</summary>
		/// <param name="a">The first, initial vector, A. Does not need to be
		/// normalized.</param>
		/// <param name="b">The second vector, B, that we're finding the 
		/// angle to. Does not need to be normalized.</param>
		/// <returns>a signed angle between two vectors in degrees! Sign will
		/// be positive if B is counter-clockwise (left) of A, and negative if
		/// B is clockwise (right) of A.</returns>
		public static float AngleBetween(Vec2 a, Vec2 b)
			=> (float)Math.Atan2(a.v.X*b.v.Y - a.v.Y*b.v.X, Vector2.Dot(a.v, b.v)) * Units.rad2deg;

		/// <summary>Calculates the distance between two points in space!
		/// Make sure they're in the same coordinate space! Uses a Sqrt, so
		/// it's not blazing fast, prefer DistanceSq when possible.</summary>
		/// <param name="a">The first point.</param>
		/// <param name="b">And the second point!</param>
		/// <returns>Distance between the two points.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Distance(Vec2 a, Vec2 b) => (b.v-a.v).Length();

		/// <summary>Calculates the distance between two points in space, but
		/// leaves them squared! Make sure they're in the same coordinate
		/// space! This is a fast function :)</summary>
		/// <param name="a">The first point.</param>
		/// <param name="b">And the second point!</param>
		/// <returns>Distance between the two points, but squared!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float DistanceSq(Vec2 a, Vec2 b)
			=> (b.v-a.v).LengthSquared();

		/// <summary>Checks if two points are within a certain radius of
		/// each other. This is an easily readable shorthand of the squared
		/// distance check. </summary>
		/// <param name="a">The first point.</param>
		/// <param name="b">And the second point!</param>
		/// <param name="radius">The distance to check against.</param>
		/// <returns>True if a and b are within radius of each other, false if 
		/// not.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static bool InRadius(Vec2 a, Vec2 b, float radius)
			=> (a.v - b.v).LengthSquared() < radius * radius;

		/// <summary>Blends (Linear Interpolation) between two vectors, based
		/// on a 'blend' value, where 0 is a, and 1 is b. Doesn't clamp 
		/// percent for you.</summary>
		/// <param name="a">First item in the blend, or '0.0' blend.</param>
		/// <param name="b">Second item in the blend, or '1.0' blend.</param>
		/// <param name="blend">A blend value between 0 and 1. Can be outside
		/// this range, it'll just interpolate outside of the a, b range.
		/// </param>
		/// <returns>An unclamped blend of a and b.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vec2 Lerp(Vec2 a, Vec2 b, float blend)
			=> Vector2.Lerp(a.v, b.v, blend);

		/// <summary>Returns a vector where each elements is the maximum 
		/// value for each corresponding pair.</summary>
		/// <param name="a">Order isn't important here.</param>
		/// <param name="b">Order isn't important here.</param>
		/// <returns>The maximum value for each corresponding vector pair.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vec2 Max(Vec2 a, Vec2 b)
			=> Vector2.Max(a.v, b.v);

		/// <summary>Returns a vector where each elements is the minimum 
		/// value for each corresponding pair.</summary>
		/// <param name="a">Order isn't important here.</param>
		/// <param name="b">Order isn't important here.</param>
		/// <returns>The minimum value for each corresponding vector pair.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vec2 Min(Vec2 a, Vec2 b)
			=> Vector2.Min(a.v, b.v);

		/// <summary>Mostly for debug purposes, this is a decent way to log or
		/// inspect the vector in debug mode. Looks like "[x, y]"</summary>
		/// <returns>A string that looks like "[x, y]"</returns>
		public override string ToString()
			=> string.Format("[{0:0.00}, {1:0.00}]", x, y);
	}
}
