namespace StereoKit
{
	/// <summary>This is a collection of vector initialization shorthands.
	/// Since math can often get a little long to write, saving a few 
	/// characters here and there can make a difference in readability. This
	/// also comes with some swizzles to make things even shorter! I also
	/// don't love the 'new' keyword on Vectors, and this eliminates that.
	/// 
	/// For example: instead of `new Vec3(2.0f, 2.0f, 2.0f)` or even
	/// `Vec3.One * 2.0f`, you could write `V.XXX(2.0f)`</summary>
	public static class V
	{
		/// <summary>Creates a Vec2, this is a straight alternative to 
		/// `new Vec2(x, y)`</summary>
		/// <param name="x">X component of the Vector</param>
		/// <param name="y">Y component of the Vector</param>
		/// <returns>A Vec2(x, y)</returns>
		public static Vec2 XY(float x, float y) => new Vec2(x, y);

		/// <summary>Creates a Vec2 where both components are the same value.
		/// This is the same as `new Vec2(x, x)`</summary>
		/// <param name="x">Both X and Y components will have this value.
		/// </param>
		/// <returns>A Vec2(x, x)</returns>
		public static Vec2 XX(float x) => new Vec2(x, x);


		/// <summary>Creates a Vec3, this is a straight alternative to 
		/// `new Vec3(x, y, z)`</summary>
		/// <param name="x">X component of the Vector</param>
		/// <param name="y">Y component of the Vector</param>
		/// <param name="z">Z component of the Vector</param>
		/// <returns>A Vec3(x, y, z)</returns>
		public static Vec3 XYZ(float x, float y, float z) => new Vec3(x, y, z);

		/// <summary>Creates a Vec3, this is a straight alternative to 
		/// `new Vec3(x, y, 0)`</summary>
		/// <param name="x">X component of the Vector</param>
		/// <param name="y">Y component of the Vector</param>
		/// <returns>A Vec3(x, y, 0)</returns>
		public static Vec3 XY0(float x, float y) => new Vec3(x, y, 0);

		/// <summary>Creates a Vec3, this is a straight alternative to 
		/// `new Vec3(x, 0, z)`</summary>
		/// <param name="x">X component of the Vector</param>
		/// <param name="z">Z component of the Vector</param>
		/// <returns>A Vec3(x, 0, z)</returns>
		public static Vec3 X0Z(float x, float z) => new Vec3(x, 0, z);

		/// <summary>Creates a Vec3 where all components are the same value.
		/// This is the same as `new Vec3(x, x, x)`</summary>
		/// <param name="x">X, Y and Z components will have this value.
		/// </param>
		/// <returns>A Vec3(x, x, x)</returns>
		public static Vec3 XXX(float x) => new Vec3(x, x, x);


		/// <summary>Creates a Vec4, this is a straight alternative to 
		/// `new Vec4(x, y, z, w)`</summary>
		/// <param name="x">X component of the Vector</param>
		/// <param name="y">Y component of the Vector</param>
		/// <param name="z">Z component of the Vector</param>
		/// <param name="w">W component of the Vector</param>
		/// <returns>A Vec4(x, y, z, w)</returns>
		public static Vec4 XYZW(float x, float y, float z, float w) => new Vec4(x, y, z, w);

		/// <summary>Creates a Vec4 where all components are the same value.
		/// This is the same as `new Vec4(x, x, x, x)`</summary>
		/// <param name="x">X, Y, Z and W components will have this value.
		/// </param>
		/// <returns>A Vec4(x, x, x, x)</returns>
		public static Vec4 XXXX(float x) => new Vec4(x, x, x, x);
	}
}
