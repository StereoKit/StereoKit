
using System;

namespace StereoKit.Swizzels
{
	/// <summary>Swizzels for Vec3</summary>
	public static class Vec3Swizzel
	{

		/// <summary>A transpose swizzle property, returns (0,x)</summary>
		public static Vec2 _X(this Vec3 value) => new Vec2(0, value.x);

		/// <summary>A transpose swizzle property, returns (0,y)</summary>
		public static Vec2 _Y(this Vec3 value) => new Vec2(0, value.y);

		/// <summary>A transpose swizzle property, returns (0,z)</summary>
		public static Vec2 _Z(this Vec3 value) => new Vec2(0, value.z);

		/// <summary>A transpose swizzle property, returns (x,0)</summary>
		public static Vec2 X_(this Vec3 value) => new Vec2(value.x, 0);

		/// <summary>A transpose swizzle property, returns (x,x)</summary>
		public static Vec2 XX(this Vec3 value) => new Vec2(value.x, value.x);

		/// <summary>A transpose swizzle property, returns (x,y)</summary>
		public static Vec2 XY(this Vec3 value) => new Vec2(value.x, value.y);

		/// <summary>A transpose swizzle property, returns (x,z)</summary>
		public static Vec2 XZ(this Vec3 value) => new Vec2(value.x, value.z);

		/// <summary>A transpose swizzle property, returns (y,0)</summary>
		public static Vec2 Y_(this Vec3 value) => new Vec2(value.y, 0);

		/// <summary>A transpose swizzle property, returns (y,x)</summary>
		public static Vec2 YX(this Vec3 value) => new Vec2(value.y, value.x);

		/// <summary>A transpose swizzle property, returns (y,y)</summary>
		public static Vec2 YY(this Vec3 value) => new Vec2(value.y, value.y);

		/// <summary>A transpose swizzle property, returns (y,z)</summary>
		public static Vec2 YZ(this Vec3 value) => new Vec2(value.y, value.z);

		/// <summary>A transpose swizzle property, returns (z,0)</summary>
		public static Vec2 Z_(this Vec3 value) => new Vec2(value.z, 0);

		/// <summary>A transpose swizzle property, returns (z,x)</summary>
		public static Vec2 ZX(this Vec3 value) => new Vec2(value.z, value.x);

		/// <summary>A transpose swizzle property, returns (z,y)</summary>
		public static Vec2 ZY(this Vec3 value) => new Vec2(value.z, value.y);

		/// <summary>A transpose swizzle property, returns (z,z)</summary>
		public static Vec2 ZZ(this Vec3 value) => new Vec2(value.z, value.z);

		/// <summary>A transpose swizzle property, returns (0,0,x)</summary>
		public static Vec3 __X(this Vec3 value) => new Vec3(0, 0, value.x);

		/// <summary>A transpose swizzle property, returns (0,0,y)</summary>
		public static Vec3 __Y(this Vec3 value) => new Vec3(0, 0, value.y);

		/// <summary>A transpose swizzle property, returns (0,0,z)</summary>
		public static Vec3 __Z(this Vec3 value) => new Vec3(0, 0, value.z);

		/// <summary>A transpose swizzle property, returns (0,x,0)</summary>
		public static Vec3 _X_(this Vec3 value) => new Vec3(0, value.x, 0);

		/// <summary>A transpose swizzle property, returns (0,x,x)</summary>
		public static Vec3 _XX(this Vec3 value) => new Vec3(0, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (0,x,y)</summary>
		public static Vec3 _XY(this Vec3 value) => new Vec3(0, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (0,x,z)</summary>
		public static Vec3 _XZ(this Vec3 value) => new Vec3(0, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (0,y,0)</summary>
		public static Vec3 _Y_(this Vec3 value) => new Vec3(0, value.y, 0);

		/// <summary>A transpose swizzle property, returns (0,y,x)</summary>
		public static Vec3 _YX(this Vec3 value) => new Vec3(0, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (0,y,y)</summary>
		public static Vec3 _YY(this Vec3 value) => new Vec3(0, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (0,y,z)</summary>
		public static Vec3 _YZ(this Vec3 value) => new Vec3(0, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (0,z,0)</summary>
		public static Vec3 _Z_(this Vec3 value) => new Vec3(0, value.z, 0);

		/// <summary>A transpose swizzle property, returns (0,z,x)</summary>
		public static Vec3 _ZX(this Vec3 value) => new Vec3(0, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (0,z,y)</summary>
		public static Vec3 _ZY(this Vec3 value) => new Vec3(0, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (0,z,z)</summary>
		public static Vec3 _ZZ(this Vec3 value) => new Vec3(0, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (x,0,0)</summary>
		public static Vec3 X__(this Vec3 value) => new Vec3(value.x, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,0,x)</summary>
		public static Vec3 X_X(this Vec3 value) => new Vec3(value.x, 0, value.x);

		/// <summary>A transpose swizzle property, returns (x,0,y)</summary>
		public static Vec3 X_Y(this Vec3 value) => new Vec3(value.x, 0, value.y);

		/// <summary>A transpose swizzle property, returns (x,0,z)</summary>
		public static Vec3 X_Z(this Vec3 value) => new Vec3(value.x, 0, value.z);

		/// <summary>A transpose swizzle property, returns (x,x,0)</summary>
		public static Vec3 XX_(this Vec3 value) => new Vec3(value.x, value.x, 0);

		/// <summary>A transpose swizzle property, returns (x,x,x)</summary>
		public static Vec3 XXX(this Vec3 value) => new Vec3(value.x, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (x,x,y)</summary>
		public static Vec3 XXY(this Vec3 value) => new Vec3(value.x, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (x,x,z)</summary>
		public static Vec3 XXZ(this Vec3 value) => new Vec3(value.x, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (x,y,0)</summary>
		public static Vec3 XY_(this Vec3 value) => new Vec3(value.x, value.y, 0);

		/// <summary>A transpose swizzle property, returns (x,y,x)</summary>
		public static Vec3 XYX(this Vec3 value) => new Vec3(value.x, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (x,y,y)</summary>
		public static Vec3 XYY(this Vec3 value) => new Vec3(value.x, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (x,y,z)</summary>
		public static Vec3 XYZ(this Vec3 value) => new Vec3(value.x, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (x,z,0)</summary>
		public static Vec3 XZ_(this Vec3 value) => new Vec3(value.x, value.z, 0);

		/// <summary>A transpose swizzle property, returns (x,z,x)</summary>
		public static Vec3 XZX(this Vec3 value) => new Vec3(value.x, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (x,z,y)</summary>
		public static Vec3 XZY(this Vec3 value) => new Vec3(value.x, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (x,z,z)</summary>
		public static Vec3 XZZ(this Vec3 value) => new Vec3(value.x, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (y,0,0)</summary>
		public static Vec3 Y__(this Vec3 value) => new Vec3(value.y, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,0,x)</summary>
		public static Vec3 Y_X(this Vec3 value) => new Vec3(value.y, 0, value.x);

		/// <summary>A transpose swizzle property, returns (y,0,y)</summary>
		public static Vec3 Y_Y(this Vec3 value) => new Vec3(value.y, 0, value.y);

		/// <summary>A transpose swizzle property, returns (y,0,z)</summary>
		public static Vec3 Y_Z(this Vec3 value) => new Vec3(value.y, 0, value.z);

		/// <summary>A transpose swizzle property, returns (y,x,0)</summary>
		public static Vec3 YX_(this Vec3 value) => new Vec3(value.y, value.x, 0);

		/// <summary>A transpose swizzle property, returns (y,x,x)</summary>
		public static Vec3 YXX(this Vec3 value) => new Vec3(value.y, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (y,x,y)</summary>
		public static Vec3 YXY(this Vec3 value) => new Vec3(value.y, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (y,x,z)</summary>
		public static Vec3 YXZ(this Vec3 value) => new Vec3(value.y, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (y,y,0)</summary>
		public static Vec3 YY_(this Vec3 value) => new Vec3(value.y, value.y, 0);

		/// <summary>A transpose swizzle property, returns (y,y,x)</summary>
		public static Vec3 YYX(this Vec3 value) => new Vec3(value.y, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (y,y,y)</summary>
		public static Vec3 YYY(this Vec3 value) => new Vec3(value.y, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (y,y,z)</summary>
		public static Vec3 YYZ(this Vec3 value) => new Vec3(value.y, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (y,z,0)</summary>
		public static Vec3 YZ_(this Vec3 value) => new Vec3(value.y, value.z, 0);

		/// <summary>A transpose swizzle property, returns (y,z,x)</summary>
		public static Vec3 YZX(this Vec3 value) => new Vec3(value.y, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (y,z,y)</summary>
		public static Vec3 YZY(this Vec3 value) => new Vec3(value.y, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (y,z,z)</summary>
		public static Vec3 YZZ(this Vec3 value) => new Vec3(value.y, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (z,0,0)</summary>
		public static Vec3 Z__(this Vec3 value) => new Vec3(value.z, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,0,x)</summary>
		public static Vec3 Z_X(this Vec3 value) => new Vec3(value.z, 0, value.x);

		/// <summary>A transpose swizzle property, returns (z,0,y)</summary>
		public static Vec3 Z_Y(this Vec3 value) => new Vec3(value.z, 0, value.y);

		/// <summary>A transpose swizzle property, returns (z,0,z)</summary>
		public static Vec3 Z_Z(this Vec3 value) => new Vec3(value.z, 0, value.z);

		/// <summary>A transpose swizzle property, returns (z,x,0)</summary>
		public static Vec3 ZX_(this Vec3 value) => new Vec3(value.z, value.x, 0);

		/// <summary>A transpose swizzle property, returns (z,x,x)</summary>
		public static Vec3 ZXX(this Vec3 value) => new Vec3(value.z, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (z,x,y)</summary>
		public static Vec3 ZXY(this Vec3 value) => new Vec3(value.z, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (z,x,z)</summary>
		public static Vec3 ZXZ(this Vec3 value) => new Vec3(value.z, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (z,y,0)</summary>
		public static Vec3 ZY_(this Vec3 value) => new Vec3(value.z, value.y, 0);

		/// <summary>A transpose swizzle property, returns (z,y,x)</summary>
		public static Vec3 ZYX(this Vec3 value) => new Vec3(value.z, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (z,y,y)</summary>
		public static Vec3 ZYY(this Vec3 value) => new Vec3(value.z, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (z,y,z)</summary>
		public static Vec3 ZYZ(this Vec3 value) => new Vec3(value.z, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (z,z,0)</summary>
		public static Vec3 ZZ_(this Vec3 value) => new Vec3(value.z, value.z, 0);

		/// <summary>A transpose swizzle property, returns (z,z,x)</summary>
		public static Vec3 ZZX(this Vec3 value) => new Vec3(value.z, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (z,z,y)</summary>
		public static Vec3 ZZY(this Vec3 value) => new Vec3(value.z, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (z,z,z)</summary>
		public static Vec3 ZZZ(this Vec3 value) => new Vec3(value.z, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (0,0,0,x)</summary>
		public static Vec4 ___X(this Vec3 value) => new Vec4(0, 0, 0, value.x);

		/// <summary>A transpose swizzle property, returns (0,0,0,y)</summary>
		public static Vec4 ___Y(this Vec3 value) => new Vec4(0, 0, 0, value.y);

		/// <summary>A transpose swizzle property, returns (0,0,0,z)</summary>
		public static Vec4 ___Z(this Vec3 value) => new Vec4(0, 0, 0, value.z);

		/// <summary>A transpose swizzle property, returns (0,0,x,0)</summary>
		public static Vec4 __X_(this Vec3 value) => new Vec4(0, 0, value.x, 0);

		/// <summary>A transpose swizzle property, returns (0,0,x,x)</summary>
		public static Vec4 __XX(this Vec3 value) => new Vec4(0, 0, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (0,0,x,y)</summary>
		public static Vec4 __XY(this Vec3 value) => new Vec4(0, 0, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (0,0,x,z)</summary>
		public static Vec4 __XZ(this Vec3 value) => new Vec4(0, 0, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (0,0,y,0)</summary>
		public static Vec4 __Y_(this Vec3 value) => new Vec4(0, 0, value.y, 0);

		/// <summary>A transpose swizzle property, returns (0,0,y,x)</summary>
		public static Vec4 __YX(this Vec3 value) => new Vec4(0, 0, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (0,0,y,y)</summary>
		public static Vec4 __YY(this Vec3 value) => new Vec4(0, 0, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (0,0,y,z)</summary>
		public static Vec4 __YZ(this Vec3 value) => new Vec4(0, 0, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (0,0,z,0)</summary>
		public static Vec4 __Z_(this Vec3 value) => new Vec4(0, 0, value.z, 0);

		/// <summary>A transpose swizzle property, returns (0,0,z,x)</summary>
		public static Vec4 __ZX(this Vec3 value) => new Vec4(0, 0, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (0,0,z,y)</summary>
		public static Vec4 __ZY(this Vec3 value) => new Vec4(0, 0, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (0,0,z,z)</summary>
		public static Vec4 __ZZ(this Vec3 value) => new Vec4(0, 0, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (0,x,0,0)</summary>
		public static Vec4 _X__(this Vec3 value) => new Vec4(0, value.x, 0, 0);

		/// <summary>A transpose swizzle property, returns (0,x,0,x)</summary>
		public static Vec4 _X_X(this Vec3 value) => new Vec4(0, value.x, 0, value.x);

		/// <summary>A transpose swizzle property, returns (0,x,0,y)</summary>
		public static Vec4 _X_Y(this Vec3 value) => new Vec4(0, value.x, 0, value.y);

		/// <summary>A transpose swizzle property, returns (0,x,0,z)</summary>
		public static Vec4 _X_Z(this Vec3 value) => new Vec4(0, value.x, 0, value.z);

		/// <summary>A transpose swizzle property, returns (0,x,x,0)</summary>
		public static Vec4 _XX_(this Vec3 value) => new Vec4(0, value.x, value.x, 0);

		/// <summary>A transpose swizzle property, returns (0,x,x,x)</summary>
		public static Vec4 _XXX(this Vec3 value) => new Vec4(0, value.x, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (0,x,x,y)</summary>
		public static Vec4 _XXY(this Vec3 value) => new Vec4(0, value.x, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (0,x,x,z)</summary>
		public static Vec4 _XXZ(this Vec3 value) => new Vec4(0, value.x, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (0,x,y,0)</summary>
		public static Vec4 _XY_(this Vec3 value) => new Vec4(0, value.x, value.y, 0);

		/// <summary>A transpose swizzle property, returns (0,x,y,x)</summary>
		public static Vec4 _XYX(this Vec3 value) => new Vec4(0, value.x, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (0,x,y,y)</summary>
		public static Vec4 _XYY(this Vec3 value) => new Vec4(0, value.x, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (0,x,y,z)</summary>
		public static Vec4 _XYZ(this Vec3 value) => new Vec4(0, value.x, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (0,x,z,0)</summary>
		public static Vec4 _XZ_(this Vec3 value) => new Vec4(0, value.x, value.z, 0);

		/// <summary>A transpose swizzle property, returns (0,x,z,x)</summary>
		public static Vec4 _XZX(this Vec3 value) => new Vec4(0, value.x, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (0,x,z,y)</summary>
		public static Vec4 _XZY(this Vec3 value) => new Vec4(0, value.x, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (0,x,z,z)</summary>
		public static Vec4 _XZZ(this Vec3 value) => new Vec4(0, value.x, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (0,y,0,0)</summary>
		public static Vec4 _Y__(this Vec3 value) => new Vec4(0, value.y, 0, 0);

		/// <summary>A transpose swizzle property, returns (0,y,0,x)</summary>
		public static Vec4 _Y_X(this Vec3 value) => new Vec4(0, value.y, 0, value.x);

		/// <summary>A transpose swizzle property, returns (0,y,0,y)</summary>
		public static Vec4 _Y_Y(this Vec3 value) => new Vec4(0, value.y, 0, value.y);

		/// <summary>A transpose swizzle property, returns (0,y,0,z)</summary>
		public static Vec4 _Y_Z(this Vec3 value) => new Vec4(0, value.y, 0, value.z);

		/// <summary>A transpose swizzle property, returns (0,y,x,0)</summary>
		public static Vec4 _YX_(this Vec3 value) => new Vec4(0, value.y, value.x, 0);

		/// <summary>A transpose swizzle property, returns (0,y,x,x)</summary>
		public static Vec4 _YXX(this Vec3 value) => new Vec4(0, value.y, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (0,y,x,y)</summary>
		public static Vec4 _YXY(this Vec3 value) => new Vec4(0, value.y, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (0,y,x,z)</summary>
		public static Vec4 _YXZ(this Vec3 value) => new Vec4(0, value.y, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (0,y,y,0)</summary>
		public static Vec4 _YY_(this Vec3 value) => new Vec4(0, value.y, value.y, 0);

		/// <summary>A transpose swizzle property, returns (0,y,y,x)</summary>
		public static Vec4 _YYX(this Vec3 value) => new Vec4(0, value.y, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (0,y,y,y)</summary>
		public static Vec4 _YYY(this Vec3 value) => new Vec4(0, value.y, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (0,y,y,z)</summary>
		public static Vec4 _YYZ(this Vec3 value) => new Vec4(0, value.y, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (0,y,z,0)</summary>
		public static Vec4 _YZ_(this Vec3 value) => new Vec4(0, value.y, value.z, 0);

		/// <summary>A transpose swizzle property, returns (0,y,z,x)</summary>
		public static Vec4 _YZX(this Vec3 value) => new Vec4(0, value.y, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (0,y,z,y)</summary>
		public static Vec4 _YZY(this Vec3 value) => new Vec4(0, value.y, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (0,y,z,z)</summary>
		public static Vec4 _YZZ(this Vec3 value) => new Vec4(0, value.y, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (0,z,0,0)</summary>
		public static Vec4 _Z__(this Vec3 value) => new Vec4(0, value.z, 0, 0);

		/// <summary>A transpose swizzle property, returns (0,z,0,x)</summary>
		public static Vec4 _Z_X(this Vec3 value) => new Vec4(0, value.z, 0, value.x);

		/// <summary>A transpose swizzle property, returns (0,z,0,y)</summary>
		public static Vec4 _Z_Y(this Vec3 value) => new Vec4(0, value.z, 0, value.y);

		/// <summary>A transpose swizzle property, returns (0,z,0,z)</summary>
		public static Vec4 _Z_Z(this Vec3 value) => new Vec4(0, value.z, 0, value.z);

		/// <summary>A transpose swizzle property, returns (0,z,x,0)</summary>
		public static Vec4 _ZX_(this Vec3 value) => new Vec4(0, value.z, value.x, 0);

		/// <summary>A transpose swizzle property, returns (0,z,x,x)</summary>
		public static Vec4 _ZXX(this Vec3 value) => new Vec4(0, value.z, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (0,z,x,y)</summary>
		public static Vec4 _ZXY(this Vec3 value) => new Vec4(0, value.z, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (0,z,x,z)</summary>
		public static Vec4 _ZXZ(this Vec3 value) => new Vec4(0, value.z, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (0,z,y,0)</summary>
		public static Vec4 _ZY_(this Vec3 value) => new Vec4(0, value.z, value.y, 0);

		/// <summary>A transpose swizzle property, returns (0,z,y,x)</summary>
		public static Vec4 _ZYX(this Vec3 value) => new Vec4(0, value.z, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (0,z,y,y)</summary>
		public static Vec4 _ZYY(this Vec3 value) => new Vec4(0, value.z, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (0,z,y,z)</summary>
		public static Vec4 _ZYZ(this Vec3 value) => new Vec4(0, value.z, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (0,z,z,0)</summary>
		public static Vec4 _ZZ_(this Vec3 value) => new Vec4(0, value.z, value.z, 0);

		/// <summary>A transpose swizzle property, returns (0,z,z,x)</summary>
		public static Vec4 _ZZX(this Vec3 value) => new Vec4(0, value.z, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (0,z,z,y)</summary>
		public static Vec4 _ZZY(this Vec3 value) => new Vec4(0, value.z, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (0,z,z,z)</summary>
		public static Vec4 _ZZZ(this Vec3 value) => new Vec4(0, value.z, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (x,0,0,0)</summary>
		public static Vec4 X___(this Vec3 value) => new Vec4(value.x, 0, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,0,0,x)</summary>
		public static Vec4 X__X(this Vec3 value) => new Vec4(value.x, 0, 0, value.x);

		/// <summary>A transpose swizzle property, returns (x,0,0,y)</summary>
		public static Vec4 X__Y(this Vec3 value) => new Vec4(value.x, 0, 0, value.y);

		/// <summary>A transpose swizzle property, returns (x,0,0,z)</summary>
		public static Vec4 X__Z(this Vec3 value) => new Vec4(value.x, 0, 0, value.z);

		/// <summary>A transpose swizzle property, returns (x,0,x,0)</summary>
		public static Vec4 X_X_(this Vec3 value) => new Vec4(value.x, 0, value.x, 0);

		/// <summary>A transpose swizzle property, returns (x,0,x,x)</summary>
		public static Vec4 X_XX(this Vec3 value) => new Vec4(value.x, 0, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (x,0,x,y)</summary>
		public static Vec4 X_XY(this Vec3 value) => new Vec4(value.x, 0, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (x,0,x,z)</summary>
		public static Vec4 X_XZ(this Vec3 value) => new Vec4(value.x, 0, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (x,0,y,0)</summary>
		public static Vec4 X_Y_(this Vec3 value) => new Vec4(value.x, 0, value.y, 0);

		/// <summary>A transpose swizzle property, returns (x,0,y,x)</summary>
		public static Vec4 X_YX(this Vec3 value) => new Vec4(value.x, 0, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (x,0,y,y)</summary>
		public static Vec4 X_YY(this Vec3 value) => new Vec4(value.x, 0, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (x,0,y,z)</summary>
		public static Vec4 X_YZ(this Vec3 value) => new Vec4(value.x, 0, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (x,0,z,0)</summary>
		public static Vec4 X_Z_(this Vec3 value) => new Vec4(value.x, 0, value.z, 0);

		/// <summary>A transpose swizzle property, returns (x,0,z,x)</summary>
		public static Vec4 X_ZX(this Vec3 value) => new Vec4(value.x, 0, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (x,0,z,y)</summary>
		public static Vec4 X_ZY(this Vec3 value) => new Vec4(value.x, 0, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (x,0,z,z)</summary>
		public static Vec4 X_ZZ(this Vec3 value) => new Vec4(value.x, 0, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (x,x,0,0)</summary>
		public static Vec4 XX__(this Vec3 value) => new Vec4(value.x, value.x, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,x,0,x)</summary>
		public static Vec4 XX_X(this Vec3 value) => new Vec4(value.x, value.x, 0, value.x);

		/// <summary>A transpose swizzle property, returns (x,x,0,y)</summary>
		public static Vec4 XX_Y(this Vec3 value) => new Vec4(value.x, value.x, 0, value.y);

		/// <summary>A transpose swizzle property, returns (x,x,0,z)</summary>
		public static Vec4 XX_Z(this Vec3 value) => new Vec4(value.x, value.x, 0, value.z);

		/// <summary>A transpose swizzle property, returns (x,x,x,0)</summary>
		public static Vec4 XXX_(this Vec3 value) => new Vec4(value.x, value.x, value.x, 0);

		/// <summary>A transpose swizzle property, returns (x,x,x,x)</summary>
		public static Vec4 XXXX(this Vec3 value) => new Vec4(value.x, value.x, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (x,x,x,y)</summary>
		public static Vec4 XXXY(this Vec3 value) => new Vec4(value.x, value.x, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (x,x,x,z)</summary>
		public static Vec4 XXXZ(this Vec3 value) => new Vec4(value.x, value.x, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (x,x,y,0)</summary>
		public static Vec4 XXY_(this Vec3 value) => new Vec4(value.x, value.x, value.y, 0);

		/// <summary>A transpose swizzle property, returns (x,x,y,x)</summary>
		public static Vec4 XXYX(this Vec3 value) => new Vec4(value.x, value.x, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (x,x,y,y)</summary>
		public static Vec4 XXYY(this Vec3 value) => new Vec4(value.x, value.x, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (x,x,y,z)</summary>
		public static Vec4 XXYZ(this Vec3 value) => new Vec4(value.x, value.x, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (x,x,z,0)</summary>
		public static Vec4 XXZ_(this Vec3 value) => new Vec4(value.x, value.x, value.z, 0);

		/// <summary>A transpose swizzle property, returns (x,x,z,x)</summary>
		public static Vec4 XXZX(this Vec3 value) => new Vec4(value.x, value.x, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (x,x,z,y)</summary>
		public static Vec4 XXZY(this Vec3 value) => new Vec4(value.x, value.x, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (x,x,z,z)</summary>
		public static Vec4 XXZZ(this Vec3 value) => new Vec4(value.x, value.x, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (x,y,0,0)</summary>
		public static Vec4 XY__(this Vec3 value) => new Vec4(value.x, value.y, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,y,0,x)</summary>
		public static Vec4 XY_X(this Vec3 value) => new Vec4(value.x, value.y, 0, value.x);

		/// <summary>A transpose swizzle property, returns (x,y,0,y)</summary>
		public static Vec4 XY_Y(this Vec3 value) => new Vec4(value.x, value.y, 0, value.y);

		/// <summary>A transpose swizzle property, returns (x,y,0,z)</summary>
		public static Vec4 XY_Z(this Vec3 value) => new Vec4(value.x, value.y, 0, value.z);

		/// <summary>A transpose swizzle property, returns (x,y,x,0)</summary>
		public static Vec4 XYX_(this Vec3 value) => new Vec4(value.x, value.y, value.x, 0);

		/// <summary>A transpose swizzle property, returns (x,y,x,x)</summary>
		public static Vec4 XYXX(this Vec3 value) => new Vec4(value.x, value.y, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (x,y,x,y)</summary>
		public static Vec4 XYXY(this Vec3 value) => new Vec4(value.x, value.y, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (x,y,x,z)</summary>
		public static Vec4 XYXZ(this Vec3 value) => new Vec4(value.x, value.y, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (x,y,y,0)</summary>
		public static Vec4 XYY_(this Vec3 value) => new Vec4(value.x, value.y, value.y, 0);

		/// <summary>A transpose swizzle property, returns (x,y,y,x)</summary>
		public static Vec4 XYYX(this Vec3 value) => new Vec4(value.x, value.y, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (x,y,y,y)</summary>
		public static Vec4 XYYY(this Vec3 value) => new Vec4(value.x, value.y, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (x,y,y,z)</summary>
		public static Vec4 XYYZ(this Vec3 value) => new Vec4(value.x, value.y, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (x,y,z,0)</summary>
		public static Vec4 XYZ_(this Vec3 value) => new Vec4(value.x, value.y, value.z, 0);

		/// <summary>A transpose swizzle property, returns (x,y,z,x)</summary>
		public static Vec4 XYZX(this Vec3 value) => new Vec4(value.x, value.y, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (x,y,z,y)</summary>
		public static Vec4 XYZY(this Vec3 value) => new Vec4(value.x, value.y, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (x,y,z,z)</summary>
		public static Vec4 XYZZ(this Vec3 value) => new Vec4(value.x, value.y, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (x,z,0,0)</summary>
		public static Vec4 XZ__(this Vec3 value) => new Vec4(value.x, value.z, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,z,0,x)</summary>
		public static Vec4 XZ_X(this Vec3 value) => new Vec4(value.x, value.z, 0, value.x);

		/// <summary>A transpose swizzle property, returns (x,z,0,y)</summary>
		public static Vec4 XZ_Y(this Vec3 value) => new Vec4(value.x, value.z, 0, value.y);

		/// <summary>A transpose swizzle property, returns (x,z,0,z)</summary>
		public static Vec4 XZ_Z(this Vec3 value) => new Vec4(value.x, value.z, 0, value.z);

		/// <summary>A transpose swizzle property, returns (x,z,x,0)</summary>
		public static Vec4 XZX_(this Vec3 value) => new Vec4(value.x, value.z, value.x, 0);

		/// <summary>A transpose swizzle property, returns (x,z,x,x)</summary>
		public static Vec4 XZXX(this Vec3 value) => new Vec4(value.x, value.z, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (x,z,x,y)</summary>
		public static Vec4 XZXY(this Vec3 value) => new Vec4(value.x, value.z, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (x,z,x,z)</summary>
		public static Vec4 XZXZ(this Vec3 value) => new Vec4(value.x, value.z, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (x,z,y,0)</summary>
		public static Vec4 XZY_(this Vec3 value) => new Vec4(value.x, value.z, value.y, 0);

		/// <summary>A transpose swizzle property, returns (x,z,y,x)</summary>
		public static Vec4 XZYX(this Vec3 value) => new Vec4(value.x, value.z, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (x,z,y,y)</summary>
		public static Vec4 XZYY(this Vec3 value) => new Vec4(value.x, value.z, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (x,z,y,z)</summary>
		public static Vec4 XZYZ(this Vec3 value) => new Vec4(value.x, value.z, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (x,z,z,0)</summary>
		public static Vec4 XZZ_(this Vec3 value) => new Vec4(value.x, value.z, value.z, 0);

		/// <summary>A transpose swizzle property, returns (x,z,z,x)</summary>
		public static Vec4 XZZX(this Vec3 value) => new Vec4(value.x, value.z, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (x,z,z,y)</summary>
		public static Vec4 XZZY(this Vec3 value) => new Vec4(value.x, value.z, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (x,z,z,z)</summary>
		public static Vec4 XZZZ(this Vec3 value) => new Vec4(value.x, value.z, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (y,0,0,0)</summary>
		public static Vec4 Y___(this Vec3 value) => new Vec4(value.y, 0, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,0,0,x)</summary>
		public static Vec4 Y__X(this Vec3 value) => new Vec4(value.y, 0, 0, value.x);

		/// <summary>A transpose swizzle property, returns (y,0,0,y)</summary>
		public static Vec4 Y__Y(this Vec3 value) => new Vec4(value.y, 0, 0, value.y);

		/// <summary>A transpose swizzle property, returns (y,0,0,z)</summary>
		public static Vec4 Y__Z(this Vec3 value) => new Vec4(value.y, 0, 0, value.z);

		/// <summary>A transpose swizzle property, returns (y,0,x,0)</summary>
		public static Vec4 Y_X_(this Vec3 value) => new Vec4(value.y, 0, value.x, 0);

		/// <summary>A transpose swizzle property, returns (y,0,x,x)</summary>
		public static Vec4 Y_XX(this Vec3 value) => new Vec4(value.y, 0, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (y,0,x,y)</summary>
		public static Vec4 Y_XY(this Vec3 value) => new Vec4(value.y, 0, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (y,0,x,z)</summary>
		public static Vec4 Y_XZ(this Vec3 value) => new Vec4(value.y, 0, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (y,0,y,0)</summary>
		public static Vec4 Y_Y_(this Vec3 value) => new Vec4(value.y, 0, value.y, 0);

		/// <summary>A transpose swizzle property, returns (y,0,y,x)</summary>
		public static Vec4 Y_YX(this Vec3 value) => new Vec4(value.y, 0, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (y,0,y,y)</summary>
		public static Vec4 Y_YY(this Vec3 value) => new Vec4(value.y, 0, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (y,0,y,z)</summary>
		public static Vec4 Y_YZ(this Vec3 value) => new Vec4(value.y, 0, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (y,0,z,0)</summary>
		public static Vec4 Y_Z_(this Vec3 value) => new Vec4(value.y, 0, value.z, 0);

		/// <summary>A transpose swizzle property, returns (y,0,z,x)</summary>
		public static Vec4 Y_ZX(this Vec3 value) => new Vec4(value.y, 0, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (y,0,z,y)</summary>
		public static Vec4 Y_ZY(this Vec3 value) => new Vec4(value.y, 0, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (y,0,z,z)</summary>
		public static Vec4 Y_ZZ(this Vec3 value) => new Vec4(value.y, 0, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (y,x,0,0)</summary>
		public static Vec4 YX__(this Vec3 value) => new Vec4(value.y, value.x, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,x,0,x)</summary>
		public static Vec4 YX_X(this Vec3 value) => new Vec4(value.y, value.x, 0, value.x);

		/// <summary>A transpose swizzle property, returns (y,x,0,y)</summary>
		public static Vec4 YX_Y(this Vec3 value) => new Vec4(value.y, value.x, 0, value.y);

		/// <summary>A transpose swizzle property, returns (y,x,0,z)</summary>
		public static Vec4 YX_Z(this Vec3 value) => new Vec4(value.y, value.x, 0, value.z);

		/// <summary>A transpose swizzle property, returns (y,x,x,0)</summary>
		public static Vec4 YXX_(this Vec3 value) => new Vec4(value.y, value.x, value.x, 0);

		/// <summary>A transpose swizzle property, returns (y,x,x,x)</summary>
		public static Vec4 YXXX(this Vec3 value) => new Vec4(value.y, value.x, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (y,x,x,y)</summary>
		public static Vec4 YXXY(this Vec3 value) => new Vec4(value.y, value.x, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (y,x,x,z)</summary>
		public static Vec4 YXXZ(this Vec3 value) => new Vec4(value.y, value.x, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (y,x,y,0)</summary>
		public static Vec4 YXY_(this Vec3 value) => new Vec4(value.y, value.x, value.y, 0);

		/// <summary>A transpose swizzle property, returns (y,x,y,x)</summary>
		public static Vec4 YXYX(this Vec3 value) => new Vec4(value.y, value.x, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (y,x,y,y)</summary>
		public static Vec4 YXYY(this Vec3 value) => new Vec4(value.y, value.x, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (y,x,y,z)</summary>
		public static Vec4 YXYZ(this Vec3 value) => new Vec4(value.y, value.x, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (y,x,z,0)</summary>
		public static Vec4 YXZ_(this Vec3 value) => new Vec4(value.y, value.x, value.z, 0);

		/// <summary>A transpose swizzle property, returns (y,x,z,x)</summary>
		public static Vec4 YXZX(this Vec3 value) => new Vec4(value.y, value.x, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (y,x,z,y)</summary>
		public static Vec4 YXZY(this Vec3 value) => new Vec4(value.y, value.x, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (y,x,z,z)</summary>
		public static Vec4 YXZZ(this Vec3 value) => new Vec4(value.y, value.x, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (y,y,0,0)</summary>
		public static Vec4 YY__(this Vec3 value) => new Vec4(value.y, value.y, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,y,0,x)</summary>
		public static Vec4 YY_X(this Vec3 value) => new Vec4(value.y, value.y, 0, value.x);

		/// <summary>A transpose swizzle property, returns (y,y,0,y)</summary>
		public static Vec4 YY_Y(this Vec3 value) => new Vec4(value.y, value.y, 0, value.y);

		/// <summary>A transpose swizzle property, returns (y,y,0,z)</summary>
		public static Vec4 YY_Z(this Vec3 value) => new Vec4(value.y, value.y, 0, value.z);

		/// <summary>A transpose swizzle property, returns (y,y,x,0)</summary>
		public static Vec4 YYX_(this Vec3 value) => new Vec4(value.y, value.y, value.x, 0);

		/// <summary>A transpose swizzle property, returns (y,y,x,x)</summary>
		public static Vec4 YYXX(this Vec3 value) => new Vec4(value.y, value.y, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (y,y,x,y)</summary>
		public static Vec4 YYXY(this Vec3 value) => new Vec4(value.y, value.y, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (y,y,x,z)</summary>
		public static Vec4 YYXZ(this Vec3 value) => new Vec4(value.y, value.y, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (y,y,y,0)</summary>
		public static Vec4 YYY_(this Vec3 value) => new Vec4(value.y, value.y, value.y, 0);

		/// <summary>A transpose swizzle property, returns (y,y,y,x)</summary>
		public static Vec4 YYYX(this Vec3 value) => new Vec4(value.y, value.y, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (y,y,y,y)</summary>
		public static Vec4 YYYY(this Vec3 value) => new Vec4(value.y, value.y, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (y,y,y,z)</summary>
		public static Vec4 YYYZ(this Vec3 value) => new Vec4(value.y, value.y, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (y,y,z,0)</summary>
		public static Vec4 YYZ_(this Vec3 value) => new Vec4(value.y, value.y, value.z, 0);

		/// <summary>A transpose swizzle property, returns (y,y,z,x)</summary>
		public static Vec4 YYZX(this Vec3 value) => new Vec4(value.y, value.y, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (y,y,z,y)</summary>
		public static Vec4 YYZY(this Vec3 value) => new Vec4(value.y, value.y, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (y,y,z,z)</summary>
		public static Vec4 YYZZ(this Vec3 value) => new Vec4(value.y, value.y, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (y,z,0,0)</summary>
		public static Vec4 YZ__(this Vec3 value) => new Vec4(value.y, value.z, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,z,0,x)</summary>
		public static Vec4 YZ_X(this Vec3 value) => new Vec4(value.y, value.z, 0, value.x);

		/// <summary>A transpose swizzle property, returns (y,z,0,y)</summary>
		public static Vec4 YZ_Y(this Vec3 value) => new Vec4(value.y, value.z, 0, value.y);

		/// <summary>A transpose swizzle property, returns (y,z,0,z)</summary>
		public static Vec4 YZ_Z(this Vec3 value) => new Vec4(value.y, value.z, 0, value.z);

		/// <summary>A transpose swizzle property, returns (y,z,x,0)</summary>
		public static Vec4 YZX_(this Vec3 value) => new Vec4(value.y, value.z, value.x, 0);

		/// <summary>A transpose swizzle property, returns (y,z,x,x)</summary>
		public static Vec4 YZXX(this Vec3 value) => new Vec4(value.y, value.z, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (y,z,x,y)</summary>
		public static Vec4 YZXY(this Vec3 value) => new Vec4(value.y, value.z, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (y,z,x,z)</summary>
		public static Vec4 YZXZ(this Vec3 value) => new Vec4(value.y, value.z, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (y,z,y,0)</summary>
		public static Vec4 YZY_(this Vec3 value) => new Vec4(value.y, value.z, value.y, 0);

		/// <summary>A transpose swizzle property, returns (y,z,y,x)</summary>
		public static Vec4 YZYX(this Vec3 value) => new Vec4(value.y, value.z, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (y,z,y,y)</summary>
		public static Vec4 YZYY(this Vec3 value) => new Vec4(value.y, value.z, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (y,z,y,z)</summary>
		public static Vec4 YZYZ(this Vec3 value) => new Vec4(value.y, value.z, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (y,z,z,0)</summary>
		public static Vec4 YZZ_(this Vec3 value) => new Vec4(value.y, value.z, value.z, 0);

		/// <summary>A transpose swizzle property, returns (y,z,z,x)</summary>
		public static Vec4 YZZX(this Vec3 value) => new Vec4(value.y, value.z, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (y,z,z,y)</summary>
		public static Vec4 YZZY(this Vec3 value) => new Vec4(value.y, value.z, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (y,z,z,z)</summary>
		public static Vec4 YZZZ(this Vec3 value) => new Vec4(value.y, value.z, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (z,0,0,0)</summary>
		public static Vec4 Z___(this Vec3 value) => new Vec4(value.z, 0, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,0,0,x)</summary>
		public static Vec4 Z__X(this Vec3 value) => new Vec4(value.z, 0, 0, value.x);

		/// <summary>A transpose swizzle property, returns (z,0,0,y)</summary>
		public static Vec4 Z__Y(this Vec3 value) => new Vec4(value.z, 0, 0, value.y);

		/// <summary>A transpose swizzle property, returns (z,0,0,z)</summary>
		public static Vec4 Z__Z(this Vec3 value) => new Vec4(value.z, 0, 0, value.z);

		/// <summary>A transpose swizzle property, returns (z,0,x,0)</summary>
		public static Vec4 Z_X_(this Vec3 value) => new Vec4(value.z, 0, value.x, 0);

		/// <summary>A transpose swizzle property, returns (z,0,x,x)</summary>
		public static Vec4 Z_XX(this Vec3 value) => new Vec4(value.z, 0, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (z,0,x,y)</summary>
		public static Vec4 Z_XY(this Vec3 value) => new Vec4(value.z, 0, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (z,0,x,z)</summary>
		public static Vec4 Z_XZ(this Vec3 value) => new Vec4(value.z, 0, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (z,0,y,0)</summary>
		public static Vec4 Z_Y_(this Vec3 value) => new Vec4(value.z, 0, value.y, 0);

		/// <summary>A transpose swizzle property, returns (z,0,y,x)</summary>
		public static Vec4 Z_YX(this Vec3 value) => new Vec4(value.z, 0, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (z,0,y,y)</summary>
		public static Vec4 Z_YY(this Vec3 value) => new Vec4(value.z, 0, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (z,0,y,z)</summary>
		public static Vec4 Z_YZ(this Vec3 value) => new Vec4(value.z, 0, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (z,0,z,0)</summary>
		public static Vec4 Z_Z_(this Vec3 value) => new Vec4(value.z, 0, value.z, 0);

		/// <summary>A transpose swizzle property, returns (z,0,z,x)</summary>
		public static Vec4 Z_ZX(this Vec3 value) => new Vec4(value.z, 0, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (z,0,z,y)</summary>
		public static Vec4 Z_ZY(this Vec3 value) => new Vec4(value.z, 0, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (z,0,z,z)</summary>
		public static Vec4 Z_ZZ(this Vec3 value) => new Vec4(value.z, 0, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (z,x,0,0)</summary>
		public static Vec4 ZX__(this Vec3 value) => new Vec4(value.z, value.x, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,x,0,x)</summary>
		public static Vec4 ZX_X(this Vec3 value) => new Vec4(value.z, value.x, 0, value.x);

		/// <summary>A transpose swizzle property, returns (z,x,0,y)</summary>
		public static Vec4 ZX_Y(this Vec3 value) => new Vec4(value.z, value.x, 0, value.y);

		/// <summary>A transpose swizzle property, returns (z,x,0,z)</summary>
		public static Vec4 ZX_Z(this Vec3 value) => new Vec4(value.z, value.x, 0, value.z);

		/// <summary>A transpose swizzle property, returns (z,x,x,0)</summary>
		public static Vec4 ZXX_(this Vec3 value) => new Vec4(value.z, value.x, value.x, 0);

		/// <summary>A transpose swizzle property, returns (z,x,x,x)</summary>
		public static Vec4 ZXXX(this Vec3 value) => new Vec4(value.z, value.x, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (z,x,x,y)</summary>
		public static Vec4 ZXXY(this Vec3 value) => new Vec4(value.z, value.x, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (z,x,x,z)</summary>
		public static Vec4 ZXXZ(this Vec3 value) => new Vec4(value.z, value.x, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (z,x,y,0)</summary>
		public static Vec4 ZXY_(this Vec3 value) => new Vec4(value.z, value.x, value.y, 0);

		/// <summary>A transpose swizzle property, returns (z,x,y,x)</summary>
		public static Vec4 ZXYX(this Vec3 value) => new Vec4(value.z, value.x, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (z,x,y,y)</summary>
		public static Vec4 ZXYY(this Vec3 value) => new Vec4(value.z, value.x, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (z,x,y,z)</summary>
		public static Vec4 ZXYZ(this Vec3 value) => new Vec4(value.z, value.x, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (z,x,z,0)</summary>
		public static Vec4 ZXZ_(this Vec3 value) => new Vec4(value.z, value.x, value.z, 0);

		/// <summary>A transpose swizzle property, returns (z,x,z,x)</summary>
		public static Vec4 ZXZX(this Vec3 value) => new Vec4(value.z, value.x, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (z,x,z,y)</summary>
		public static Vec4 ZXZY(this Vec3 value) => new Vec4(value.z, value.x, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (z,x,z,z)</summary>
		public static Vec4 ZXZZ(this Vec3 value) => new Vec4(value.z, value.x, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (z,y,0,0)</summary>
		public static Vec4 ZY__(this Vec3 value) => new Vec4(value.z, value.y, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,y,0,x)</summary>
		public static Vec4 ZY_X(this Vec3 value) => new Vec4(value.z, value.y, 0, value.x);

		/// <summary>A transpose swizzle property, returns (z,y,0,y)</summary>
		public static Vec4 ZY_Y(this Vec3 value) => new Vec4(value.z, value.y, 0, value.y);

		/// <summary>A transpose swizzle property, returns (z,y,0,z)</summary>
		public static Vec4 ZY_Z(this Vec3 value) => new Vec4(value.z, value.y, 0, value.z);

		/// <summary>A transpose swizzle property, returns (z,y,x,0)</summary>
		public static Vec4 ZYX_(this Vec3 value) => new Vec4(value.z, value.y, value.x, 0);

		/// <summary>A transpose swizzle property, returns (z,y,x,x)</summary>
		public static Vec4 ZYXX(this Vec3 value) => new Vec4(value.z, value.y, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (z,y,x,y)</summary>
		public static Vec4 ZYXY(this Vec3 value) => new Vec4(value.z, value.y, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (z,y,x,z)</summary>
		public static Vec4 ZYXZ(this Vec3 value) => new Vec4(value.z, value.y, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (z,y,y,0)</summary>
		public static Vec4 ZYY_(this Vec3 value) => new Vec4(value.z, value.y, value.y, 0);

		/// <summary>A transpose swizzle property, returns (z,y,y,x)</summary>
		public static Vec4 ZYYX(this Vec3 value) => new Vec4(value.z, value.y, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (z,y,y,y)</summary>
		public static Vec4 ZYYY(this Vec3 value) => new Vec4(value.z, value.y, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (z,y,y,z)</summary>
		public static Vec4 ZYYZ(this Vec3 value) => new Vec4(value.z, value.y, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (z,y,z,0)</summary>
		public static Vec4 ZYZ_(this Vec3 value) => new Vec4(value.z, value.y, value.z, 0);

		/// <summary>A transpose swizzle property, returns (z,y,z,x)</summary>
		public static Vec4 ZYZX(this Vec3 value) => new Vec4(value.z, value.y, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (z,y,z,y)</summary>
		public static Vec4 ZYZY(this Vec3 value) => new Vec4(value.z, value.y, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (z,y,z,z)</summary>
		public static Vec4 ZYZZ(this Vec3 value) => new Vec4(value.z, value.y, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (z,z,0,0)</summary>
		public static Vec4 ZZ__(this Vec3 value) => new Vec4(value.z, value.z, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,z,0,x)</summary>
		public static Vec4 ZZ_X(this Vec3 value) => new Vec4(value.z, value.z, 0, value.x);

		/// <summary>A transpose swizzle property, returns (z,z,0,y)</summary>
		public static Vec4 ZZ_Y(this Vec3 value) => new Vec4(value.z, value.z, 0, value.y);

		/// <summary>A transpose swizzle property, returns (z,z,0,z)</summary>
		public static Vec4 ZZ_Z(this Vec3 value) => new Vec4(value.z, value.z, 0, value.z);

		/// <summary>A transpose swizzle property, returns (z,z,x,0)</summary>
		public static Vec4 ZZX_(this Vec3 value) => new Vec4(value.z, value.z, value.x, 0);

		/// <summary>A transpose swizzle property, returns (z,z,x,x)</summary>
		public static Vec4 ZZXX(this Vec3 value) => new Vec4(value.z, value.z, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (z,z,x,y)</summary>
		public static Vec4 ZZXY(this Vec3 value) => new Vec4(value.z, value.z, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (z,z,x,z)</summary>
		public static Vec4 ZZXZ(this Vec3 value) => new Vec4(value.z, value.z, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (z,z,y,0)</summary>
		public static Vec4 ZZY_(this Vec3 value) => new Vec4(value.z, value.z, value.y, 0);

		/// <summary>A transpose swizzle property, returns (z,z,y,x)</summary>
		public static Vec4 ZZYX(this Vec3 value) => new Vec4(value.z, value.z, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (z,z,y,y)</summary>
		public static Vec4 ZZYY(this Vec3 value) => new Vec4(value.z, value.z, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (z,z,y,z)</summary>
		public static Vec4 ZZYZ(this Vec3 value) => new Vec4(value.z, value.z, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (z,z,z,0)</summary>
		public static Vec4 ZZZ_(this Vec3 value) => new Vec4(value.z, value.z, value.z, 0);

		/// <summary>A transpose swizzle property, returns (z,z,z,x)</summary>
		public static Vec4 ZZZX(this Vec3 value) => new Vec4(value.z, value.z, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (z,z,z,y)</summary>
		public static Vec4 ZZZY(this Vec3 value) => new Vec4(value.z, value.z, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (z,z,z,z)</summary>
		public static Vec4 ZZZZ(this Vec3 value) => new Vec4(value.z, value.z, value.z, value.z);

	}
}
