using System;

namespace StereoKit.Swizzels
{
	/// <summary>Swizzels to expand float</summary>
	public static class FloatSwizzel
	{

		/// <summary>A transpose swizzle property, returns (0,x)</summary>
		public static Vec2 _X(this float x) => new Vec2(0, x);

		/// <summary>A transpose swizzle property, returns (x,0)</summary>
		public static Vec2 X_(this float x) => new Vec2(x, 0);

		/// <summary>A transpose swizzle property, returns (x,x)</summary>
		public static Vec2 XX(this float x) => new Vec2(x, x);

		/// <summary>A transpose swizzle property, returns (0,0,x)</summary>
		public static Vec3 __X(this float x) => new Vec3(0, 0, x);

		/// <summary>A transpose swizzle property, returns (0,x,0)</summary>
		public static Vec3 _X_(this float x) => new Vec3(0, x, 0);

		/// <summary>A transpose swizzle property, returns (0,x,x)</summary>
		public static Vec3 _XX(this float x) => new Vec3(0, x, x);

		/// <summary>A transpose swizzle property, returns (x,0,0)</summary>
		public static Vec3 X__(this float x) => new Vec3(x, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,0,x)</summary>
		public static Vec3 X_X(this float x) => new Vec3(x, 0, x);

		/// <summary>A transpose swizzle property, returns (x,x,0)</summary>
		public static Vec3 XX_(this float x) => new Vec3(x, x, 0);

		/// <summary>A transpose swizzle property, returns (x,x,x)</summary>
		public static Vec3 XXX(this float x) => new Vec3(x, x, x);

		/// <summary>A transpose swizzle property, returns (0,0,0,x)</summary>
		public static Vec4 ___X(this float x) => new Vec4(0, 0, 0, x);

		/// <summary>A transpose swizzle property, returns (0,0,x,0)</summary>
		public static Vec4 __X_(this float x) => new Vec4(0, 0, x, 0);

		/// <summary>A transpose swizzle property, returns (0,0,x,x)</summary>
		public static Vec4 __XX(this float x) => new Vec4(0, 0, x, x);

		/// <summary>A transpose swizzle property, returns (0,x,0,0)</summary>
		public static Vec4 _X__(this float x) => new Vec4(0, x, 0, 0);

		/// <summary>A transpose swizzle property, returns (0,x,0,x)</summary>
		public static Vec4 _X_X(this float x) => new Vec4(0, x, 0, x);

		/// <summary>A transpose swizzle property, returns (0,x,x,0)</summary>
		public static Vec4 _XX_(this float x) => new Vec4(0, x, x, 0);

		/// <summary>A transpose swizzle property, returns (0,x,x,x)</summary>
		public static Vec4 _XXX(this float x) => new Vec4(0, x, x, x);

		/// <summary>A transpose swizzle property, returns (x,0,0,0)</summary>
		public static Vec4 X___(this float x) => new Vec4(x, 0, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,0,0,x)</summary>
		public static Vec4 X__X(this float x) => new Vec4(x, 0, 0, x);

		/// <summary>A transpose swizzle property, returns (x,0,x,0)</summary>
		public static Vec4 X_X_(this float x) => new Vec4(x, 0, x, 0);

		/// <summary>A transpose swizzle property, returns (x,0,x,x)</summary>
		public static Vec4 X_XX(this float x) => new Vec4(x, 0, x, x);

		/// <summary>A transpose swizzle property, returns (x,x,0,0)</summary>
		public static Vec4 XX__(this float x) => new Vec4(x, x, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,x,0,x)</summary>
		public static Vec4 XX_X(this float x) => new Vec4(x, x, 0, x);

		/// <summary>A transpose swizzle property, returns (x,x,x,0)</summary>
		public static Vec4 XXX_(this float x) => new Vec4(x, x, x, 0);

		/// <summary>A transpose swizzle property, returns (x,x,x,x)</summary>
		public static Vec4 XXXX(this float x) => new Vec4(x, x, x, x);

	}
}
