using System.Numerics;
using System.Runtime.CompilerServices;

namespace StereoKit
{
	/// <summary>A vector with 4 components: x, y, z, and w. Can be useful
	/// for things like shaders, where the registers are aligned to 4 float
	/// vectors.
	/// 
	/// This is a wrapper on System.Numerics.Vector4, so it's SIMD optimized,
	/// and can be cast to and from implicitly.</summary>
	public struct Vec4
	{
		/// <summary>The internal, wrapped System.Numerics type. This can be
		/// nice to have around so you can pass its fields as 'ref', which
		/// you can't do with properties. You won't often need this, as
		/// implicit conversions to System.Numerics types are also
		/// provided.</summary>
		public Vector4 v;
		public float x { get => v.X; set => v.X = value; }
		public float y { get => v.Y; set => v.Y = value; }
		public float z { get => v.Z; set => v.Z = value; }
		public float w { get => v.W; set => v.W = value; }

		/// <summary>A normalized Vector that points down the X axis, this is
		/// the same as `new Vec4(1,0,0,0)`.</summary>
		public static readonly Vec4 UnitX = new Vec4(1, 0, 0, 0);
		/// <summary>A normalized Vector that points down the Y axis, this is
		/// the same as `new Vec4(0,1,0,0)`.</summary>
		public static readonly Vec4 UnitY = new Vec4(0, 1, 0, 0);
		/// <summary>A normalized Vector that points down the Z axis, this is
		/// the same as `new Vec4(0,1,0,0)`.</summary>
		public static readonly Vec4 UnitZ = new Vec4(0, 0, 1, 0);
		/// <summary>A normalized Vector that points down the W axis, this is
		/// the same as `new Vec4(0,1,0,0)`.</summary>
		public static readonly Vec4 UnitW = new Vec4(0, 0, 0, 1);

		/// <summary>This extracts a Vec2 from the X and Y axes.</summary>
		public Vec2 XY  { get => new Vec2(x, y); set { x = value.x; y = value.y; } }
		/// <summary>This extracts a Vec2 from the Y and Z axes.</summary>
		public Vec2 YZ  { get => new Vec2(y, z); set { y = value.x; z = value.y; } }
		/// <summary>This extracts a Vec2 from the Z and W axes.</summary>
		public Vec2 ZW  { get => new Vec2(z, w); set { z = value.x; w = value.y; } }
		/// <summary>This extracts a Vec2 from the X and Z axes.</summary>
		public Vec2 XZ  { get => new Vec2(x, z); set { x = value.x; z = value.y; } }
		/// <summary>This extracts a Vec3 from the X, Y, and Z axes.</summary>
		public Vec3 XYZ { get => new Vec3(x, y, z); set { x = value.x; y = value.y; z = value.z; } }

		/// <summary>A Vec4 and a Quat are only really different by name and
		/// purpose. So, if you need to do Quat math with your Vec4, or visa
		/// versa, who am I to judge?</summary>
		public Quat Quat => new Quat(x, y, z, w);

		/// <summary>A basic constructor, just copies the values in!</summary>
		/// <param name="x">X component of the vector.</param>
		/// <param name="y">Y component of the vector.</param>
		/// <param name="z">Z component of the vector.</param>
		/// <param name="w">W component of the vector.</param>
		public Vec4(float x, float y, float z, float w) 
			=> v = new Vector4(x, y, z, w);

		/// <summary>A basic constructor, just copies the values in!</summary>
		/// <param name="xyz">X, Y and Z components of the vector.</param>
		/// <param name="w">W component of the vector.</param>
		public Vec4(Vec3 xyz, float w)
			=> v = new Vector4(xyz.x, xyz.y, xyz.z, w);

		/// <summary>A basic constructor, just copies the values in!</summary>
		/// <param name="xy">X and Y components of the vector.</param>
		/// <param name="zw">Z and W components of the vector.</param>
		public Vec4(Vec2 xy, Vec2 zw)
			=> v = new Vector4(xy.x, xy.y, zw.x, zw.y);

		public static implicit operator Vec4(Vector4 v) => new Vec4(v.X, v.Y, v.Z, v.W);
		public static implicit operator Vector4(Vec4 v) => v.v;

		public static Vec4 operator +(Vec4 a, Vec4 b) => a.v + b.v;
		public static Vec4 operator -(Vec4 a, Vec4 b) => a.v - b.v;
		public static Vec4 operator *(Vec4 b, Vec4 a) => a.v * b.v;
		public static Vec4 operator /(Vec4 b, Vec4 a) => a.v / b.v;
		public static Vec4 operator -(Vec4 a) => -a.v;
		public static Vec4 operator *(Vec4 a, float b) => a.v * b;
		public static Vec4 operator /(Vec4 a, float b) => a.v / b;
		public static Vec4 operator *(float b, Vec4 a) => a.v * b;

		/// <summary>What's a dot product do for 4D vectors, you might ask?
		/// Well, I'm no mathematician, so hopefully you are! I've never used
		/// it before. Whatever you're doing with this function, it's SIMD 
		/// fast!</summary>
		/// <param name="a">First vector.</param>
		/// <param name="b">Second vector.</param>
		/// <returns>The dot product!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Dot(Vec4 a, Vec4 b)
			=> Vector4.Dot(a.v, b.v);

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
		public static Vec4 Lerp(Vec4 a, Vec4 b, float blend)
			=> Vector4.Lerp(a.v, b.v, blend);

		/// <summary>Returns a vector where each elements is the maximum 
		/// value for each corresponding pair.</summary>
		/// <param name="a">Order isn't important here.</param>
		/// <param name="b">Order isn't important here.</param>
		/// <returns>The maximum value for each corresponding vector pair.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vec4 Max(Vec4 a, Vec4 b)
			=> Vector4.Max(a.v, b.v);

		/// <summary>Returns a vector where each elements is the minimum 
		/// value for each corresponding pair.</summary>
		/// <param name="a">Order isn't important here.</param>
		/// <param name="b">Order isn't important here.</param>
		/// <returns>The minimum value for each corresponding vector pair.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vec4 Min(Vec4 a, Vec4 b)
			=> Vector4.Min(a.v, b.v);

		public override string ToString()
			=> string.Format("[{0:0.00}, {1:0.00}, {2:0.00}, {3:0.00}]", x, y, z, w);
	}
}