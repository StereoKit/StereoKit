using System.Numerics;
using System.Runtime.CompilerServices;

namespace StereoKit
{
	/// <summary>A vector with 3 components: x, y, z. This can represent a 
	/// point in space, a directional vector, or any other sort of value with
	/// 3 dimensions to it!
	/// 
	/// StereoKit uses a right-handed coordinate system, where +x is to the
	/// right, +y is upwards, and -z is forward.</summary>
	public struct Vec3
	{
		/// <summary>The internal, wrapped System.Numerics type. This can be
		/// nice to have around so you can pass its fields as 'ref', which
		/// you can't do with properties. You won't often need this, as
		/// implicit conversions to System.Numerics types are also
		/// provided.</summary>
		public Vector3 v;
		/// <summary>X component.</summary>
		public float x { get => v.X; set => v.X=value; }
		/// <summary>Y component.</summary>
		public float y { get => v.Y; set => v.Y=value; }
		/// <summary>Z component.</summary>
		public float z { get => v.Z; set => v.Z=value; }

		/// <summary>Creates a vector from x, y, and z values! StereoKit uses
		/// a right-handed metric coordinate system, where +x is to the
		/// right, +y is upwards, and -z is forward.</summary>
		/// <param name="x">The x axis.</param>
		/// <param name="y">The y axis.</param>
		/// <param name="z">The z axis.</param>
		public Vec3(float x, float y, float z) => v = new Vector3(x, y, z);

		/// <summary>Creates a vector with all values the same! StereoKit uses
		/// a right-handed metric coordinate system, where +x is to the
		/// right, +y is upwards, and -z is forward.</summary>
		/// <param name="xyz">The x,y,and z axis.</param>
		public Vec3(float xyz) => v = new Vector3(xyz, xyz, xyz);

		/// <summary>Allows implicit conversion from System.Numerics.Vector3
		/// to StereoKit.Vec3.</summary>
		/// <param name="v">Any System.Numerics Vector3.</param>
		/// <returns>A StereoKit compatible vector.</returns>
		public static implicit operator Vec3(Vector3 v) => new Vec3(v.X, v.Y, v.Z);
		/// <summary>Allows implicit conversion from StereoKit.Vec3 to
		/// System.Numerics.Vector3</summary>
		/// <param name="v">Any StereoKit.Vec3.</param>
		/// <returns>A System.Numerics compatible vector.</returns>
		public static implicit operator Vector3(Vec3 v) => v.v;

		/// <summary>Adds matching components together. Commutative.</summary>
		/// <param name="a">Any vector.</param>
		/// <param name="b">Any vector.</param>
		/// <returns>A new vector from the added components.</returns>
		public static Vec3 operator +(Vec3 a, Vec3 b) => a.v + b.v;
		/// <summary>Subtracts matching components from eachother. Not
		/// commutative.</summary>
		/// <param name="a">Any vector.</param>
		/// <param name="b">Any vector.</param>
		/// <returns>A new vector from the subtracted components.</returns>
		public static Vec3 operator -(Vec3 a, Vec3 b) => a.v - b.v;
		/// <summary>A component-wise vector multiplication, same thing as
		/// a non-uniform scale. NOT a dot or cross product! Commutative.
		/// </summary>
		/// <param name="a">Any vector.</param>
		/// <param name="b">Any vector.</param>
		/// <returns>A new vector a scaled by b.</returns>
		public static Vec3 operator *(Vec3 a, Vec3 b) => a.v * b.v;
		/// <summary>A component-wise vector division. Not commutative</summary>
		/// <param name="a">Any vector.</param>
		/// <param name="b">Any vector.</param>
		/// <returns>A new vector a divided by b.</returns>
		public static Vec3 operator /(Vec3 a, Vec3 b) => a.v / b.v;
		/// <summary>Vector negation, returns a vector where each component has
		/// been negated.</summary>
		/// <param name="a">Any vector.</param>
		/// <returns>A vector where each component has been negated.</returns>
		public static Vec3 operator -(Vec3 a) => -a.v;
		/// <summary>A scalar vector multiplication.</summary>
		/// <param name="a">Any vector.</param>
		/// <param name="b">Any scalar.</param>
		/// <returns>A new vector a scaled by b.</returns>
		public static Vec3 operator *(Vec3 a, float b) => a.v * b;
		/// <summary>A scalar vector division.</summary>
		/// <param name="a">Any vector.</param>
		/// <param name="b">Any scalar.</param>
		/// <returns>A new vector a divided by b.</returns>
		public static Vec3 operator /(Vec3 a, float b) => a.v / b;
		/// <summary>A scalar vector multiplication.</summary>
		/// <param name="a">Any scalar.</param>
		/// <param name="b">Any vector.</param>
		/// <returns>A new vector a scaled by b.</returns>
		public static Vec3 operator *(float a, Vec3 b) => b.v * a;

		/// <summary>Shorthand for a vector where all values are 0! Same as
		/// `new Vec3(0,0,0)`.</summary>
		public static readonly Vec3 Zero = new Vec3(0, 0, 0);
		/// <summary>Shorthand for a vector where all values are 1! Same as
		/// `new Vec3(1,1,1)`.</summary>
		public static readonly Vec3 One = new Vec3(1, 1, 1);
		/// <summary>A vector representing the up axis. In StereoKit, this is
		/// the same as `new Vec3(0,1,0)`.</summary>
		public static readonly Vec3 Up = new Vec3(0, 1, 0);
		/// <summary>StereoKit uses a right-handed coordinate system, which
		/// means that forward is looking down the -Z axis! This value is the
		/// same as `new Vec3(0,0,-1)`. This is NOT the same as UnitZ!
		/// </summary>
		public static readonly Vec3 Forward = new Vec3(0, 0, -1);
		/// <summary>When looking forward, this is the direction to the
		/// right! In StereoKit, this is the same as `new Vec3(1,0,0)`
		/// </summary>
		public static readonly Vec3 Right = new Vec3(1, 0, 0);
		/// <summary>A normalized Vector that points down the X axis, this is
		/// the same as `new Vec3(1,0,0)`.</summary>
		public static readonly Vec3 UnitX = new Vec3(1, 0, 0);
		/// <summary>A normalized Vector that points down the Y axis, this is
		/// the same as `new Vec3(0,1,0)`.</summary>
		public static readonly Vec3 UnitY = new Vec3(0, 1, 0);
		/// <summary>A normalized Vector that points down the Z axis, this is
		/// the same as `new Vec3(0,0,1)`. This is NOT the same as Forward!
		/// </summary>
		public static readonly Vec3 UnitZ = new Vec3(0, 0, 1);

		/// <summary>This extracts a Vec2 from the X and Y axes.</summary>
		public Vec2 XY { get => new Vec2(x, y); set { x = value.x; y = value.y; } }
		/// <summary>This extracts a Vec2 from the Y and Z axes.</summary>
		public Vec2 YZ { get => new Vec2(y, z); set { y = value.x; z = value.y; } }
		/// <summary>This extracts a Vec2 from the X and Z axes.</summary>
		public Vec2 XZ { get => new Vec2(x, z); set { x = value.x; z = value.y; } }
		/// <summary>This returns a Vec3 that has been flattened to 0 on the
		/// Y axis. No other changes are made.</summary>
		public Vec3 X0Z => new Vec3(x, 0, z);
		/// <summary>This returns a Vec3 that has been flattened to 0 on the
		/// Z axis. No other changes are made.</summary>
		public Vec3 XY0 => new Vec3(x, y, 0);

		/// <summary>Magnitude is the length of the vector! The distance from
		/// the origin to this point. Uses Math.Sqrt, so it's not dirt cheap
		/// or anything.</summary>
		public float Magnitude => v.Length();
		/// <summary>This is the length, or magnitude of the vector! The
		/// distance from the origin to this point. Uses Math.Sqrt, so it's
		/// not dirt cheap or anything.</summary>
		public float Length => v.Length();

		/// <summary>This is the squared magnitude of the vector! It skips
		/// the Sqrt call, and just gives you the squared version for speedy
		/// calculations that can work with it squared.</summary>
		public float MagnitudeSq => v.LengthSquared();
		/// <summary>This is the squared length of the vector! It skips the
		/// Sqrt call, and just gives you the squared version for speedy
		/// calculations that can work with it squared.</summary>
		public float LengthSq => v.LengthSquared();

		/// <summary>Creates a normalized vector (vector with a length of 1)
		/// from the current vector. Will not work properly if the vector has
		/// a length of zero.</summary>
		/// <returns>The normalized (length of 1) vector!</returns>
		public Vec3 Normalized => v / v.Length();

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
		public static float Dot(Vec3 a, Vec3 b)
			=> Vector3.Dot(a.v, b.v);

		/// <summary>The cross product of two vectors!</summary>
		/// <param name="a">First vector!</param>
		/// <param name="b">Second vector!</param>
		/// <returns>Result is -not- a unit vector, even if both 'a' and 'b' 
		/// are unit vectors.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vec3 Cross(Vec3 a, Vec3 b)
			=> Vector3.Cross(a.v, b.v);

		/// <summary>Turns this vector into a normalized vector (vector with
		/// a length of 1) from the current vector. Will not work properly if
		/// the vector has a length of zero.</summary>
		/// <returns>The normalized (length of 1) vector!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void Normalize() => v = v / v.Length();

		/// <summary>Checks if a point is within a certain radius of this one.
		/// This is an easily readable shorthand of the squared distance check.
		/// </summary>
		/// <param name="pt">The point to check against.</param>
		/// <param name="radius">The distance to check against.</param>
		/// <returns>True if the points are within radius of each other, false
		/// not.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool InRadius(Vec3 pt, float radius)
			=> (v - pt.v).LengthSquared() < radius * radius;

		/// <summary>Calculates the distance between two points in space!
		/// Make sure they're in the same coordinate space! Uses a Sqrt, so
		/// it's not blazing fast, prefer DistanceSq when possible.</summary>
		/// <param name="a">The first point.</param>
		/// <param name="b">And the second point!</param>
		/// <returns>Distance between the two points.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Distance(Vec3 a, Vec3 b)
			=> (a.v-b.v).Length();

		/// <summary>Calculates the distance between two points in space, but
		/// leaves them squared! Make sure they're in the same coordinate
		/// space! This is a fast function :)</summary>
		/// <param name="a">The first point.</param>
		/// <param name="b">And the second point!</param>
		/// <returns>Distance between the two points, but squared!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float DistanceSq(Vec3 a, Vec3 b)
			=> (a.v-b.v).LengthSquared();

		/// <summary>Checks if two points are within a certain radius of
		/// each other. This is an easily readable shorthand of the squared
		/// distance check. </summary>
		/// <param name="a">The first point.</param>
		/// <param name="b">And the second point!</param>
		/// <param name="radius">The distance to check against.</param>
		/// <returns>True if a and b are within radius of each other, false if
		/// not.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static bool InRadius(Vec3 a, Vec3 b, float radius)
			=> (a.v-b.v).LengthSquared() < radius * radius;

		/// <summary>Creates a vector that points out at the given 2D angle!
		/// This creates the vector on the XZ plane, and allows you to
		/// specify a constant y value.</summary>
		/// <param name="angleDeg">Angle in degrees, starting from (1,0) at
		/// 0, and continuing to (0,1) at 90.</param>
		/// <param name="y">A constant value you can assign to the resulting
		/// vector's y component.</param>
		/// <returns>A Vector pointing at the given angle! If y is zero, this
		/// will be a normalized vector (vector with a length of 1).</returns>
		public static Vec3 AngleXZ(float angleDeg, float y = 0)
			=> new Vec3(SKMath.Cos(angleDeg * Units.deg2rad), y, SKMath.Sin(angleDeg * Units.deg2rad));

		/// <summary>Creates a vector that points out at the given 2D angle!
		/// This creates the vector on the XY plane, and allows you to
		/// specify a constant z value.</summary>
		/// <param name="angleDeg">Angle in degrees, starting from (1,0) at
		/// 0, and continuing to (0,1) at 90.</param>
		/// <param name="z">A constant value you can assign to the resulting
		/// vector's z component.</param>
		/// <returns>A vector pointing at the given angle! If z is zero, this
		/// will be a normalized vector (vector with a length of 1).</returns>
		public static Vec3 AngleXY(float angleDeg, float z = 0)
			=> new Vec3(SKMath.Cos(angleDeg * Units.deg2rad), SKMath.Sin(angleDeg * Units.deg2rad), z);

		/// <summary>Calculates the angle between two vectors in degrees!
		/// Vectors do not need to be normalized, and the result will always be
		/// positive.</summary>
		/// <param name="a">The first, initial vector, A. Does not need to be
		/// normalized.</param>
		/// <param name="b">The second vector, B, that we're finding the
		/// angle to. Does not need to be normalized.</param>
		/// <returns>A positive angle between two vectors in degrees!</returns>
		public static float AngleBetween(Vec3 a, Vec3 b)
			=> (float)System.Math.Acos( Dot(a, b) / SKMath.Sqrt(a.LengthSq * b.LengthSq) ) * Units.rad2deg;

		/// <summary>Exactly the same as Vec3.Cross, but has some naming
		/// mnemonics for getting the order right when trying to find a
		/// perpendicular vector using the cross product. This'll also make
		/// it more obvious to read if that's what you're actually going for
		/// when crossing vectors!
		/// 
		/// If you consider a forward vector and an up vector, then the
		/// direction to the right is pretty trivial to imagine in relation
		/// to those vectors!</summary>
		/// <param name="forward">What way are you facing?</param>
		/// <param name="up">What direction is up?</param>
		/// <returns>Your direction to the right! Result is -not- a unit
		/// vector, even if both 'forward'
		/// and 'up' are unit vectors.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vec3 PerpendicularRight(Vec3 forward, Vec3 up)
			=> Vector3.Cross(forward.v, up.v);

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
		public static Vec3 Lerp(Vec3 a, Vec3 b, float blend)
			=> Vector3.Lerp(a.v, b.v, blend);

		/// <summary>Returns a vector where each elements is the maximum
		/// value for each corresponding pair.</summary>
		/// <param name="a">Order isn't important here.</param>
		/// <param name="b">Order isn't important here.</param>
		/// <returns>The maximum value for each corresponding vector pair.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vec3 Max(Vec3 a, Vec3 b)
			=> Vector3.Max(a.v, b.v);

		/// <summary>Returns a vector where each elements is the minimum
		/// value for each corresponding pair.</summary>
		/// <param name="a">Order isn't important here.</param>
		/// <param name="b">Order isn't important here.</param>
		/// <returns>The minimum value for each corresponding vector pair.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vec3 Min(Vec3 a, Vec3 b)
			=> Vector3.Min(a.v, b.v);

		/// <summary>Mostly for debug purposes, this is a decent way to log or
		/// inspect the vector in debug mode. Looks like "[x, y, z]"</summary>
		/// <returns>A string that looks like "[x, y, z]"</returns>
		public override string ToString()
			=> string.Format("[{0:0.##}, {1:0.##}, {2:0.##}]", x, y, z);
	}
}
