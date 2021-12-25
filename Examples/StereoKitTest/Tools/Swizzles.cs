using System;

namespace StereoKit.Swizzels
{

	/// <summary>Swizzels for Vec4</summary>
	public static class Vec4Swizzel
	{

		/// <summary>A transpose swizzle property, returns (0,x)</summary>
		public static Vec2 _X(this Vec4 value) => new Vec2(0, value.x);

		/// <summary>A transpose swizzle property, returns (0,y)</summary>
		public static Vec2 _Y(this Vec4 value) => new Vec2(0, value.y);

		/// <summary>A transpose swizzle property, returns (0,z)</summary>
		public static Vec2 _Z(this Vec4 value) => new Vec2(0, value.z);

		/// <summary>A transpose swizzle property, returns (0,w)</summary>
		public static Vec2 _W(this Vec4 value) => new Vec2(0, value.w);

		/// <summary>A transpose swizzle property, returns (x,0)</summary>
		public static Vec2 X_(this Vec4 value) => new Vec2(value.x, 0);

		/// <summary>A transpose swizzle property, returns (x,x)</summary>
		public static Vec2 XX(this Vec4 value) => new Vec2(value.x, value.x);

		/// <summary>A transpose swizzle property, returns (x,y)</summary>
		public static Vec2 XY(this Vec4 value) => new Vec2(value.x, value.y);

		/// <summary>A transpose swizzle property, returns (x,z)</summary>
		public static Vec2 XZ(this Vec4 value) => new Vec2(value.x, value.z);

		/// <summary>A transpose swizzle property, returns (x,w)</summary>
		public static Vec2 XW(this Vec4 value) => new Vec2(value.x, value.w);

		/// <summary>A transpose swizzle property, returns (y,0)</summary>
		public static Vec2 Y_(this Vec4 value) => new Vec2(value.y, 0);

		/// <summary>A transpose swizzle property, returns (y,x)</summary>
		public static Vec2 YX(this Vec4 value) => new Vec2(value.y, value.x);

		/// <summary>A transpose swizzle property, returns (y,y)</summary>
		public static Vec2 YY(this Vec4 value) => new Vec2(value.y, value.y);

		/// <summary>A transpose swizzle property, returns (y,z)</summary>
		public static Vec2 YZ(this Vec4 value) => new Vec2(value.y, value.z);

		/// <summary>A transpose swizzle property, returns (y,w)</summary>
		public static Vec2 YW(this Vec4 value) => new Vec2(value.y, value.w);

		/// <summary>A transpose swizzle property, returns (z,0)</summary>
		public static Vec2 Z_(this Vec4 value) => new Vec2(value.z, 0);

		/// <summary>A transpose swizzle property, returns (z,x)</summary>
		public static Vec2 ZX(this Vec4 value) => new Vec2(value.z, value.x);

		/// <summary>A transpose swizzle property, returns (z,y)</summary>
		public static Vec2 ZY(this Vec4 value) => new Vec2(value.z, value.y);

		/// <summary>A transpose swizzle property, returns (z,z)</summary>
		public static Vec2 ZZ(this Vec4 value) => new Vec2(value.z, value.z);

		/// <summary>A transpose swizzle property, returns (z,w)</summary>
		public static Vec2 ZW(this Vec4 value) => new Vec2(value.z, value.w);

		/// <summary>A transpose swizzle property, returns (w,0)</summary>
		public static Vec2 W_(this Vec4 value) => new Vec2(value.w, 0);

		/// <summary>A transpose swizzle property, returns (w,x)</summary>
		public static Vec2 WX(this Vec4 value) => new Vec2(value.w, value.x);

		/// <summary>A transpose swizzle property, returns (w,y)</summary>
		public static Vec2 WY(this Vec4 value) => new Vec2(value.w, value.y);

		/// <summary>A transpose swizzle property, returns (w,z)</summary>
		public static Vec2 WZ(this Vec4 value) => new Vec2(value.w, value.z);

		/// <summary>A transpose swizzle property, returns (w,w)</summary>
		public static Vec2 WW(this Vec4 value) => new Vec2(value.w, value.w);

		/// <summary>A transpose swizzle property, returns (0,0,x)</summary>
		public static Vec3 __X(this Vec4 value) => new Vec3(0, 0, value.x);

		/// <summary>A transpose swizzle property, returns (0,0,y)</summary>
		public static Vec3 __Y(this Vec4 value) => new Vec3(0, 0, value.y);

		/// <summary>A transpose swizzle property, returns (0,0,z)</summary>
		public static Vec3 __Z(this Vec4 value) => new Vec3(0, 0, value.z);

		/// <summary>A transpose swizzle property, returns (0,0,w)</summary>
		public static Vec3 __W(this Vec4 value) => new Vec3(0, 0, value.w);

		/// <summary>A transpose swizzle property, returns (0,x,0)</summary>
		public static Vec3 _X_(this Vec4 value) => new Vec3(0, value.x, 0);

		/// <summary>A transpose swizzle property, returns (0,x,x)</summary>
		public static Vec3 _XX(this Vec4 value) => new Vec3(0, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (0,x,y)</summary>
		public static Vec3 _XY(this Vec4 value) => new Vec3(0, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (0,x,z)</summary>
		public static Vec3 _XZ(this Vec4 value) => new Vec3(0, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (0,x,w)</summary>
		public static Vec3 _XW(this Vec4 value) => new Vec3(0, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (0,y,0)</summary>
		public static Vec3 _Y_(this Vec4 value) => new Vec3(0, value.y, 0);

		/// <summary>A transpose swizzle property, returns (0,y,x)</summary>
		public static Vec3 _YX(this Vec4 value) => new Vec3(0, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (0,y,y)</summary>
		public static Vec3 _YY(this Vec4 value) => new Vec3(0, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (0,y,z)</summary>
		public static Vec3 _YZ(this Vec4 value) => new Vec3(0, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (0,y,w)</summary>
		public static Vec3 _YW(this Vec4 value) => new Vec3(0, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (0,z,0)</summary>
		public static Vec3 _Z_(this Vec4 value) => new Vec3(0, value.z, 0);

		/// <summary>A transpose swizzle property, returns (0,z,x)</summary>
		public static Vec3 _ZX(this Vec4 value) => new Vec3(0, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (0,z,y)</summary>
		public static Vec3 _ZY(this Vec4 value) => new Vec3(0, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (0,z,z)</summary>
		public static Vec3 _ZZ(this Vec4 value) => new Vec3(0, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (0,z,w)</summary>
		public static Vec3 _ZW(this Vec4 value) => new Vec3(0, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (0,w,0)</summary>
		public static Vec3 _W_(this Vec4 value) => new Vec3(0, value.w, 0);

		/// <summary>A transpose swizzle property, returns (0,w,x)</summary>
		public static Vec3 _WX(this Vec4 value) => new Vec3(0, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (0,w,y)</summary>
		public static Vec3 _WY(this Vec4 value) => new Vec3(0, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (0,w,z)</summary>
		public static Vec3 _WZ(this Vec4 value) => new Vec3(0, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (0,w,w)</summary>
		public static Vec3 _WW(this Vec4 value) => new Vec3(0, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (x,0,0)</summary>
		public static Vec3 X__(this Vec4 value) => new Vec3(value.x, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,0,x)</summary>
		public static Vec3 X_X(this Vec4 value) => new Vec3(value.x, 0, value.x);

		/// <summary>A transpose swizzle property, returns (x,0,y)</summary>
		public static Vec3 X_Y(this Vec4 value) => new Vec3(value.x, 0, value.y);

		/// <summary>A transpose swizzle property, returns (x,0,z)</summary>
		public static Vec3 X_Z(this Vec4 value) => new Vec3(value.x, 0, value.z);

		/// <summary>A transpose swizzle property, returns (x,0,w)</summary>
		public static Vec3 X_W(this Vec4 value) => new Vec3(value.x, 0, value.w);

		/// <summary>A transpose swizzle property, returns (x,x,0)</summary>
		public static Vec3 XX_(this Vec4 value) => new Vec3(value.x, value.x, 0);

		/// <summary>A transpose swizzle property, returns (x,x,x)</summary>
		public static Vec3 XXX(this Vec4 value) => new Vec3(value.x, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (x,x,y)</summary>
		public static Vec3 XXY(this Vec4 value) => new Vec3(value.x, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (x,x,z)</summary>
		public static Vec3 XXZ(this Vec4 value) => new Vec3(value.x, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (x,x,w)</summary>
		public static Vec3 XXW(this Vec4 value) => new Vec3(value.x, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (x,y,0)</summary>
		public static Vec3 XY_(this Vec4 value) => new Vec3(value.x, value.y, 0);

		/// <summary>A transpose swizzle property, returns (x,y,x)</summary>
		public static Vec3 XYX(this Vec4 value) => new Vec3(value.x, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (x,y,y)</summary>
		public static Vec3 XYY(this Vec4 value) => new Vec3(value.x, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (x,y,z)</summary>
		public static Vec3 XYZ(this Vec4 value) => new Vec3(value.x, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (x,y,w)</summary>
		public static Vec3 XYW(this Vec4 value) => new Vec3(value.x, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (x,z,0)</summary>
		public static Vec3 XZ_(this Vec4 value) => new Vec3(value.x, value.z, 0);

		/// <summary>A transpose swizzle property, returns (x,z,x)</summary>
		public static Vec3 XZX(this Vec4 value) => new Vec3(value.x, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (x,z,y)</summary>
		public static Vec3 XZY(this Vec4 value) => new Vec3(value.x, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (x,z,z)</summary>
		public static Vec3 XZZ(this Vec4 value) => new Vec3(value.x, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (x,z,w)</summary>
		public static Vec3 XZW(this Vec4 value) => new Vec3(value.x, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (x,w,0)</summary>
		public static Vec3 XW_(this Vec4 value) => new Vec3(value.x, value.w, 0);

		/// <summary>A transpose swizzle property, returns (x,w,x)</summary>
		public static Vec3 XWX(this Vec4 value) => new Vec3(value.x, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (x,w,y)</summary>
		public static Vec3 XWY(this Vec4 value) => new Vec3(value.x, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (x,w,z)</summary>
		public static Vec3 XWZ(this Vec4 value) => new Vec3(value.x, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (x,w,w)</summary>
		public static Vec3 XWW(this Vec4 value) => new Vec3(value.x, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (y,0,0)</summary>
		public static Vec3 Y__(this Vec4 value) => new Vec3(value.y, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,0,x)</summary>
		public static Vec3 Y_X(this Vec4 value) => new Vec3(value.y, 0, value.x);

		/// <summary>A transpose swizzle property, returns (y,0,y)</summary>
		public static Vec3 Y_Y(this Vec4 value) => new Vec3(value.y, 0, value.y);

		/// <summary>A transpose swizzle property, returns (y,0,z)</summary>
		public static Vec3 Y_Z(this Vec4 value) => new Vec3(value.y, 0, value.z);

		/// <summary>A transpose swizzle property, returns (y,0,w)</summary>
		public static Vec3 Y_W(this Vec4 value) => new Vec3(value.y, 0, value.w);

		/// <summary>A transpose swizzle property, returns (y,x,0)</summary>
		public static Vec3 YX_(this Vec4 value) => new Vec3(value.y, value.x, 0);

		/// <summary>A transpose swizzle property, returns (y,x,x)</summary>
		public static Vec3 YXX(this Vec4 value) => new Vec3(value.y, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (y,x,y)</summary>
		public static Vec3 YXY(this Vec4 value) => new Vec3(value.y, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (y,x,z)</summary>
		public static Vec3 YXZ(this Vec4 value) => new Vec3(value.y, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (y,x,w)</summary>
		public static Vec3 YXW(this Vec4 value) => new Vec3(value.y, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (y,y,0)</summary>
		public static Vec3 YY_(this Vec4 value) => new Vec3(value.y, value.y, 0);

		/// <summary>A transpose swizzle property, returns (y,y,x)</summary>
		public static Vec3 YYX(this Vec4 value) => new Vec3(value.y, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (y,y,y)</summary>
		public static Vec3 YYY(this Vec4 value) => new Vec3(value.y, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (y,y,z)</summary>
		public static Vec3 YYZ(this Vec4 value) => new Vec3(value.y, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (y,y,w)</summary>
		public static Vec3 YYW(this Vec4 value) => new Vec3(value.y, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (y,z,0)</summary>
		public static Vec3 YZ_(this Vec4 value) => new Vec3(value.y, value.z, 0);

		/// <summary>A transpose swizzle property, returns (y,z,x)</summary>
		public static Vec3 YZX(this Vec4 value) => new Vec3(value.y, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (y,z,y)</summary>
		public static Vec3 YZY(this Vec4 value) => new Vec3(value.y, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (y,z,z)</summary>
		public static Vec3 YZZ(this Vec4 value) => new Vec3(value.y, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (y,z,w)</summary>
		public static Vec3 YZW(this Vec4 value) => new Vec3(value.y, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (y,w,0)</summary>
		public static Vec3 YW_(this Vec4 value) => new Vec3(value.y, value.w, 0);

		/// <summary>A transpose swizzle property, returns (y,w,x)</summary>
		public static Vec3 YWX(this Vec4 value) => new Vec3(value.y, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (y,w,y)</summary>
		public static Vec3 YWY(this Vec4 value) => new Vec3(value.y, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (y,w,z)</summary>
		public static Vec3 YWZ(this Vec4 value) => new Vec3(value.y, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (y,w,w)</summary>
		public static Vec3 YWW(this Vec4 value) => new Vec3(value.y, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (z,0,0)</summary>
		public static Vec3 Z__(this Vec4 value) => new Vec3(value.z, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,0,x)</summary>
		public static Vec3 Z_X(this Vec4 value) => new Vec3(value.z, 0, value.x);

		/// <summary>A transpose swizzle property, returns (z,0,y)</summary>
		public static Vec3 Z_Y(this Vec4 value) => new Vec3(value.z, 0, value.y);

		/// <summary>A transpose swizzle property, returns (z,0,z)</summary>
		public static Vec3 Z_Z(this Vec4 value) => new Vec3(value.z, 0, value.z);

		/// <summary>A transpose swizzle property, returns (z,0,w)</summary>
		public static Vec3 Z_W(this Vec4 value) => new Vec3(value.z, 0, value.w);

		/// <summary>A transpose swizzle property, returns (z,x,0)</summary>
		public static Vec3 ZX_(this Vec4 value) => new Vec3(value.z, value.x, 0);

		/// <summary>A transpose swizzle property, returns (z,x,x)</summary>
		public static Vec3 ZXX(this Vec4 value) => new Vec3(value.z, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (z,x,y)</summary>
		public static Vec3 ZXY(this Vec4 value) => new Vec3(value.z, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (z,x,z)</summary>
		public static Vec3 ZXZ(this Vec4 value) => new Vec3(value.z, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (z,x,w)</summary>
		public static Vec3 ZXW(this Vec4 value) => new Vec3(value.z, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (z,y,0)</summary>
		public static Vec3 ZY_(this Vec4 value) => new Vec3(value.z, value.y, 0);

		/// <summary>A transpose swizzle property, returns (z,y,x)</summary>
		public static Vec3 ZYX(this Vec4 value) => new Vec3(value.z, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (z,y,y)</summary>
		public static Vec3 ZYY(this Vec4 value) => new Vec3(value.z, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (z,y,z)</summary>
		public static Vec3 ZYZ(this Vec4 value) => new Vec3(value.z, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (z,y,w)</summary>
		public static Vec3 ZYW(this Vec4 value) => new Vec3(value.z, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (z,z,0)</summary>
		public static Vec3 ZZ_(this Vec4 value) => new Vec3(value.z, value.z, 0);

		/// <summary>A transpose swizzle property, returns (z,z,x)</summary>
		public static Vec3 ZZX(this Vec4 value) => new Vec3(value.z, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (z,z,y)</summary>
		public static Vec3 ZZY(this Vec4 value) => new Vec3(value.z, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (z,z,z)</summary>
		public static Vec3 ZZZ(this Vec4 value) => new Vec3(value.z, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (z,z,w)</summary>
		public static Vec3 ZZW(this Vec4 value) => new Vec3(value.z, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (z,w,0)</summary>
		public static Vec3 ZW_(this Vec4 value) => new Vec3(value.z, value.w, 0);

		/// <summary>A transpose swizzle property, returns (z,w,x)</summary>
		public static Vec3 ZWX(this Vec4 value) => new Vec3(value.z, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (z,w,y)</summary>
		public static Vec3 ZWY(this Vec4 value) => new Vec3(value.z, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (z,w,z)</summary>
		public static Vec3 ZWZ(this Vec4 value) => new Vec3(value.z, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (z,w,w)</summary>
		public static Vec3 ZWW(this Vec4 value) => new Vec3(value.z, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (w,0,0)</summary>
		public static Vec3 W__(this Vec4 value) => new Vec3(value.w, 0, 0);

		/// <summary>A transpose swizzle property, returns (w,0,x)</summary>
		public static Vec3 W_X(this Vec4 value) => new Vec3(value.w, 0, value.x);

		/// <summary>A transpose swizzle property, returns (w,0,y)</summary>
		public static Vec3 W_Y(this Vec4 value) => new Vec3(value.w, 0, value.y);

		/// <summary>A transpose swizzle property, returns (w,0,z)</summary>
		public static Vec3 W_Z(this Vec4 value) => new Vec3(value.w, 0, value.z);

		/// <summary>A transpose swizzle property, returns (w,0,w)</summary>
		public static Vec3 W_W(this Vec4 value) => new Vec3(value.w, 0, value.w);

		/// <summary>A transpose swizzle property, returns (w,x,0)</summary>
		public static Vec3 WX_(this Vec4 value) => new Vec3(value.w, value.x, 0);

		/// <summary>A transpose swizzle property, returns (w,x,x)</summary>
		public static Vec3 WXX(this Vec4 value) => new Vec3(value.w, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (w,x,y)</summary>
		public static Vec3 WXY(this Vec4 value) => new Vec3(value.w, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (w,x,z)</summary>
		public static Vec3 WXZ(this Vec4 value) => new Vec3(value.w, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (w,x,w)</summary>
		public static Vec3 WXW(this Vec4 value) => new Vec3(value.w, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (w,y,0)</summary>
		public static Vec3 WY_(this Vec4 value) => new Vec3(value.w, value.y, 0);

		/// <summary>A transpose swizzle property, returns (w,y,x)</summary>
		public static Vec3 WYX(this Vec4 value) => new Vec3(value.w, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (w,y,y)</summary>
		public static Vec3 WYY(this Vec4 value) => new Vec3(value.w, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (w,y,z)</summary>
		public static Vec3 WYZ(this Vec4 value) => new Vec3(value.w, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (w,y,w)</summary>
		public static Vec3 WYW(this Vec4 value) => new Vec3(value.w, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (w,z,0)</summary>
		public static Vec3 WZ_(this Vec4 value) => new Vec3(value.w, value.z, 0);

		/// <summary>A transpose swizzle property, returns (w,z,x)</summary>
		public static Vec3 WZX(this Vec4 value) => new Vec3(value.w, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (w,z,y)</summary>
		public static Vec3 WZY(this Vec4 value) => new Vec3(value.w, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (w,z,z)</summary>
		public static Vec3 WZZ(this Vec4 value) => new Vec3(value.w, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (w,z,w)</summary>
		public static Vec3 WZW(this Vec4 value) => new Vec3(value.w, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (w,w,0)</summary>
		public static Vec3 WW_(this Vec4 value) => new Vec3(value.w, value.w, 0);

		/// <summary>A transpose swizzle property, returns (w,w,x)</summary>
		public static Vec3 WWX(this Vec4 value) => new Vec3(value.w, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (w,w,y)</summary>
		public static Vec3 WWY(this Vec4 value) => new Vec3(value.w, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (w,w,z)</summary>
		public static Vec3 WWZ(this Vec4 value) => new Vec3(value.w, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (w,w,w)</summary>
		public static Vec3 WWW(this Vec4 value) => new Vec3(value.w, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (0,0,0,x)</summary>
		public static Vec4 ___X(this Vec4 value) => new Vec4(0, 0, 0, value.x);

		/// <summary>A transpose swizzle property, returns (0,0,0,y)</summary>
		public static Vec4 ___Y(this Vec4 value) => new Vec4(0, 0, 0, value.y);

		/// <summary>A transpose swizzle property, returns (0,0,0,z)</summary>
		public static Vec4 ___Z(this Vec4 value) => new Vec4(0, 0, 0, value.z);

		/// <summary>A transpose swizzle property, returns (0,0,0,w)</summary>
		public static Vec4 ___W(this Vec4 value) => new Vec4(0, 0, 0, value.w);

		/// <summary>A transpose swizzle property, returns (0,0,x,0)</summary>
		public static Vec4 __X_(this Vec4 value) => new Vec4(0, 0, value.x, 0);

		/// <summary>A transpose swizzle property, returns (0,0,x,x)</summary>
		public static Vec4 __XX(this Vec4 value) => new Vec4(0, 0, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (0,0,x,y)</summary>
		public static Vec4 __XY(this Vec4 value) => new Vec4(0, 0, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (0,0,x,z)</summary>
		public static Vec4 __XZ(this Vec4 value) => new Vec4(0, 0, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (0,0,x,w)</summary>
		public static Vec4 __XW(this Vec4 value) => new Vec4(0, 0, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (0,0,y,0)</summary>
		public static Vec4 __Y_(this Vec4 value) => new Vec4(0, 0, value.y, 0);

		/// <summary>A transpose swizzle property, returns (0,0,y,x)</summary>
		public static Vec4 __YX(this Vec4 value) => new Vec4(0, 0, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (0,0,y,y)</summary>
		public static Vec4 __YY(this Vec4 value) => new Vec4(0, 0, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (0,0,y,z)</summary>
		public static Vec4 __YZ(this Vec4 value) => new Vec4(0, 0, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (0,0,y,w)</summary>
		public static Vec4 __YW(this Vec4 value) => new Vec4(0, 0, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (0,0,z,0)</summary>
		public static Vec4 __Z_(this Vec4 value) => new Vec4(0, 0, value.z, 0);

		/// <summary>A transpose swizzle property, returns (0,0,z,x)</summary>
		public static Vec4 __ZX(this Vec4 value) => new Vec4(0, 0, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (0,0,z,y)</summary>
		public static Vec4 __ZY(this Vec4 value) => new Vec4(0, 0, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (0,0,z,z)</summary>
		public static Vec4 __ZZ(this Vec4 value) => new Vec4(0, 0, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (0,0,z,w)</summary>
		public static Vec4 __ZW(this Vec4 value) => new Vec4(0, 0, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (0,0,w,0)</summary>
		public static Vec4 __W_(this Vec4 value) => new Vec4(0, 0, value.w, 0);

		/// <summary>A transpose swizzle property, returns (0,0,w,x)</summary>
		public static Vec4 __WX(this Vec4 value) => new Vec4(0, 0, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (0,0,w,y)</summary>
		public static Vec4 __WY(this Vec4 value) => new Vec4(0, 0, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (0,0,w,z)</summary>
		public static Vec4 __WZ(this Vec4 value) => new Vec4(0, 0, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (0,0,w,w)</summary>
		public static Vec4 __WW(this Vec4 value) => new Vec4(0, 0, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (0,x,0,0)</summary>
		public static Vec4 _X__(this Vec4 value) => new Vec4(0, value.x, 0, 0);

		/// <summary>A transpose swizzle property, returns (0,x,0,x)</summary>
		public static Vec4 _X_X(this Vec4 value) => new Vec4(0, value.x, 0, value.x);

		/// <summary>A transpose swizzle property, returns (0,x,0,y)</summary>
		public static Vec4 _X_Y(this Vec4 value) => new Vec4(0, value.x, 0, value.y);

		/// <summary>A transpose swizzle property, returns (0,x,0,z)</summary>
		public static Vec4 _X_Z(this Vec4 value) => new Vec4(0, value.x, 0, value.z);

		/// <summary>A transpose swizzle property, returns (0,x,0,w)</summary>
		public static Vec4 _X_W(this Vec4 value) => new Vec4(0, value.x, 0, value.w);

		/// <summary>A transpose swizzle property, returns (0,x,x,0)</summary>
		public static Vec4 _XX_(this Vec4 value) => new Vec4(0, value.x, value.x, 0);

		/// <summary>A transpose swizzle property, returns (0,x,x,x)</summary>
		public static Vec4 _XXX(this Vec4 value) => new Vec4(0, value.x, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (0,x,x,y)</summary>
		public static Vec4 _XXY(this Vec4 value) => new Vec4(0, value.x, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (0,x,x,z)</summary>
		public static Vec4 _XXZ(this Vec4 value) => new Vec4(0, value.x, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (0,x,x,w)</summary>
		public static Vec4 _XXW(this Vec4 value) => new Vec4(0, value.x, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (0,x,y,0)</summary>
		public static Vec4 _XY_(this Vec4 value) => new Vec4(0, value.x, value.y, 0);

		/// <summary>A transpose swizzle property, returns (0,x,y,x)</summary>
		public static Vec4 _XYX(this Vec4 value) => new Vec4(0, value.x, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (0,x,y,y)</summary>
		public static Vec4 _XYY(this Vec4 value) => new Vec4(0, value.x, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (0,x,y,z)</summary>
		public static Vec4 _XYZ(this Vec4 value) => new Vec4(0, value.x, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (0,x,y,w)</summary>
		public static Vec4 _XYW(this Vec4 value) => new Vec4(0, value.x, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (0,x,z,0)</summary>
		public static Vec4 _XZ_(this Vec4 value) => new Vec4(0, value.x, value.z, 0);

		/// <summary>A transpose swizzle property, returns (0,x,z,x)</summary>
		public static Vec4 _XZX(this Vec4 value) => new Vec4(0, value.x, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (0,x,z,y)</summary>
		public static Vec4 _XZY(this Vec4 value) => new Vec4(0, value.x, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (0,x,z,z)</summary>
		public static Vec4 _XZZ(this Vec4 value) => new Vec4(0, value.x, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (0,x,z,w)</summary>
		public static Vec4 _XZW(this Vec4 value) => new Vec4(0, value.x, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (0,x,w,0)</summary>
		public static Vec4 _XW_(this Vec4 value) => new Vec4(0, value.x, value.w, 0);

		/// <summary>A transpose swizzle property, returns (0,x,w,x)</summary>
		public static Vec4 _XWX(this Vec4 value) => new Vec4(0, value.x, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (0,x,w,y)</summary>
		public static Vec4 _XWY(this Vec4 value) => new Vec4(0, value.x, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (0,x,w,z)</summary>
		public static Vec4 _XWZ(this Vec4 value) => new Vec4(0, value.x, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (0,x,w,w)</summary>
		public static Vec4 _XWW(this Vec4 value) => new Vec4(0, value.x, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (0,y,0,0)</summary>
		public static Vec4 _Y__(this Vec4 value) => new Vec4(0, value.y, 0, 0);

		/// <summary>A transpose swizzle property, returns (0,y,0,x)</summary>
		public static Vec4 _Y_X(this Vec4 value) => new Vec4(0, value.y, 0, value.x);

		/// <summary>A transpose swizzle property, returns (0,y,0,y)</summary>
		public static Vec4 _Y_Y(this Vec4 value) => new Vec4(0, value.y, 0, value.y);

		/// <summary>A transpose swizzle property, returns (0,y,0,z)</summary>
		public static Vec4 _Y_Z(this Vec4 value) => new Vec4(0, value.y, 0, value.z);

		/// <summary>A transpose swizzle property, returns (0,y,0,w)</summary>
		public static Vec4 _Y_W(this Vec4 value) => new Vec4(0, value.y, 0, value.w);

		/// <summary>A transpose swizzle property, returns (0,y,x,0)</summary>
		public static Vec4 _YX_(this Vec4 value) => new Vec4(0, value.y, value.x, 0);

		/// <summary>A transpose swizzle property, returns (0,y,x,x)</summary>
		public static Vec4 _YXX(this Vec4 value) => new Vec4(0, value.y, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (0,y,x,y)</summary>
		public static Vec4 _YXY(this Vec4 value) => new Vec4(0, value.y, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (0,y,x,z)</summary>
		public static Vec4 _YXZ(this Vec4 value) => new Vec4(0, value.y, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (0,y,x,w)</summary>
		public static Vec4 _YXW(this Vec4 value) => new Vec4(0, value.y, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (0,y,y,0)</summary>
		public static Vec4 _YY_(this Vec4 value) => new Vec4(0, value.y, value.y, 0);

		/// <summary>A transpose swizzle property, returns (0,y,y,x)</summary>
		public static Vec4 _YYX(this Vec4 value) => new Vec4(0, value.y, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (0,y,y,y)</summary>
		public static Vec4 _YYY(this Vec4 value) => new Vec4(0, value.y, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (0,y,y,z)</summary>
		public static Vec4 _YYZ(this Vec4 value) => new Vec4(0, value.y, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (0,y,y,w)</summary>
		public static Vec4 _YYW(this Vec4 value) => new Vec4(0, value.y, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (0,y,z,0)</summary>
		public static Vec4 _YZ_(this Vec4 value) => new Vec4(0, value.y, value.z, 0);

		/// <summary>A transpose swizzle property, returns (0,y,z,x)</summary>
		public static Vec4 _YZX(this Vec4 value) => new Vec4(0, value.y, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (0,y,z,y)</summary>
		public static Vec4 _YZY(this Vec4 value) => new Vec4(0, value.y, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (0,y,z,z)</summary>
		public static Vec4 _YZZ(this Vec4 value) => new Vec4(0, value.y, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (0,y,z,w)</summary>
		public static Vec4 _YZW(this Vec4 value) => new Vec4(0, value.y, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (0,y,w,0)</summary>
		public static Vec4 _YW_(this Vec4 value) => new Vec4(0, value.y, value.w, 0);

		/// <summary>A transpose swizzle property, returns (0,y,w,x)</summary>
		public static Vec4 _YWX(this Vec4 value) => new Vec4(0, value.y, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (0,y,w,y)</summary>
		public static Vec4 _YWY(this Vec4 value) => new Vec4(0, value.y, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (0,y,w,z)</summary>
		public static Vec4 _YWZ(this Vec4 value) => new Vec4(0, value.y, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (0,y,w,w)</summary>
		public static Vec4 _YWW(this Vec4 value) => new Vec4(0, value.y, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (0,z,0,0)</summary>
		public static Vec4 _Z__(this Vec4 value) => new Vec4(0, value.z, 0, 0);

		/// <summary>A transpose swizzle property, returns (0,z,0,x)</summary>
		public static Vec4 _Z_X(this Vec4 value) => new Vec4(0, value.z, 0, value.x);

		/// <summary>A transpose swizzle property, returns (0,z,0,y)</summary>
		public static Vec4 _Z_Y(this Vec4 value) => new Vec4(0, value.z, 0, value.y);

		/// <summary>A transpose swizzle property, returns (0,z,0,z)</summary>
		public static Vec4 _Z_Z(this Vec4 value) => new Vec4(0, value.z, 0, value.z);

		/// <summary>A transpose swizzle property, returns (0,z,0,w)</summary>
		public static Vec4 _Z_W(this Vec4 value) => new Vec4(0, value.z, 0, value.w);

		/// <summary>A transpose swizzle property, returns (0,z,x,0)</summary>
		public static Vec4 _ZX_(this Vec4 value) => new Vec4(0, value.z, value.x, 0);

		/// <summary>A transpose swizzle property, returns (0,z,x,x)</summary>
		public static Vec4 _ZXX(this Vec4 value) => new Vec4(0, value.z, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (0,z,x,y)</summary>
		public static Vec4 _ZXY(this Vec4 value) => new Vec4(0, value.z, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (0,z,x,z)</summary>
		public static Vec4 _ZXZ(this Vec4 value) => new Vec4(0, value.z, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (0,z,x,w)</summary>
		public static Vec4 _ZXW(this Vec4 value) => new Vec4(0, value.z, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (0,z,y,0)</summary>
		public static Vec4 _ZY_(this Vec4 value) => new Vec4(0, value.z, value.y, 0);

		/// <summary>A transpose swizzle property, returns (0,z,y,x)</summary>
		public static Vec4 _ZYX(this Vec4 value) => new Vec4(0, value.z, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (0,z,y,y)</summary>
		public static Vec4 _ZYY(this Vec4 value) => new Vec4(0, value.z, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (0,z,y,z)</summary>
		public static Vec4 _ZYZ(this Vec4 value) => new Vec4(0, value.z, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (0,z,y,w)</summary>
		public static Vec4 _ZYW(this Vec4 value) => new Vec4(0, value.z, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (0,z,z,0)</summary>
		public static Vec4 _ZZ_(this Vec4 value) => new Vec4(0, value.z, value.z, 0);

		/// <summary>A transpose swizzle property, returns (0,z,z,x)</summary>
		public static Vec4 _ZZX(this Vec4 value) => new Vec4(0, value.z, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (0,z,z,y)</summary>
		public static Vec4 _ZZY(this Vec4 value) => new Vec4(0, value.z, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (0,z,z,z)</summary>
		public static Vec4 _ZZZ(this Vec4 value) => new Vec4(0, value.z, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (0,z,z,w)</summary>
		public static Vec4 _ZZW(this Vec4 value) => new Vec4(0, value.z, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (0,z,w,0)</summary>
		public static Vec4 _ZW_(this Vec4 value) => new Vec4(0, value.z, value.w, 0);

		/// <summary>A transpose swizzle property, returns (0,z,w,x)</summary>
		public static Vec4 _ZWX(this Vec4 value) => new Vec4(0, value.z, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (0,z,w,y)</summary>
		public static Vec4 _ZWY(this Vec4 value) => new Vec4(0, value.z, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (0,z,w,z)</summary>
		public static Vec4 _ZWZ(this Vec4 value) => new Vec4(0, value.z, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (0,z,w,w)</summary>
		public static Vec4 _ZWW(this Vec4 value) => new Vec4(0, value.z, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (0,w,0,0)</summary>
		public static Vec4 _W__(this Vec4 value) => new Vec4(0, value.w, 0, 0);

		/// <summary>A transpose swizzle property, returns (0,w,0,x)</summary>
		public static Vec4 _W_X(this Vec4 value) => new Vec4(0, value.w, 0, value.x);

		/// <summary>A transpose swizzle property, returns (0,w,0,y)</summary>
		public static Vec4 _W_Y(this Vec4 value) => new Vec4(0, value.w, 0, value.y);

		/// <summary>A transpose swizzle property, returns (0,w,0,z)</summary>
		public static Vec4 _W_Z(this Vec4 value) => new Vec4(0, value.w, 0, value.z);

		/// <summary>A transpose swizzle property, returns (0,w,0,w)</summary>
		public static Vec4 _W_W(this Vec4 value) => new Vec4(0, value.w, 0, value.w);

		/// <summary>A transpose swizzle property, returns (0,w,x,0)</summary>
		public static Vec4 _WX_(this Vec4 value) => new Vec4(0, value.w, value.x, 0);

		/// <summary>A transpose swizzle property, returns (0,w,x,x)</summary>
		public static Vec4 _WXX(this Vec4 value) => new Vec4(0, value.w, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (0,w,x,y)</summary>
		public static Vec4 _WXY(this Vec4 value) => new Vec4(0, value.w, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (0,w,x,z)</summary>
		public static Vec4 _WXZ(this Vec4 value) => new Vec4(0, value.w, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (0,w,x,w)</summary>
		public static Vec4 _WXW(this Vec4 value) => new Vec4(0, value.w, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (0,w,y,0)</summary>
		public static Vec4 _WY_(this Vec4 value) => new Vec4(0, value.w, value.y, 0);

		/// <summary>A transpose swizzle property, returns (0,w,y,x)</summary>
		public static Vec4 _WYX(this Vec4 value) => new Vec4(0, value.w, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (0,w,y,y)</summary>
		public static Vec4 _WYY(this Vec4 value) => new Vec4(0, value.w, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (0,w,y,z)</summary>
		public static Vec4 _WYZ(this Vec4 value) => new Vec4(0, value.w, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (0,w,y,w)</summary>
		public static Vec4 _WYW(this Vec4 value) => new Vec4(0, value.w, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (0,w,z,0)</summary>
		public static Vec4 _WZ_(this Vec4 value) => new Vec4(0, value.w, value.z, 0);

		/// <summary>A transpose swizzle property, returns (0,w,z,x)</summary>
		public static Vec4 _WZX(this Vec4 value) => new Vec4(0, value.w, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (0,w,z,y)</summary>
		public static Vec4 _WZY(this Vec4 value) => new Vec4(0, value.w, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (0,w,z,z)</summary>
		public static Vec4 _WZZ(this Vec4 value) => new Vec4(0, value.w, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (0,w,z,w)</summary>
		public static Vec4 _WZW(this Vec4 value) => new Vec4(0, value.w, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (0,w,w,0)</summary>
		public static Vec4 _WW_(this Vec4 value) => new Vec4(0, value.w, value.w, 0);

		/// <summary>A transpose swizzle property, returns (0,w,w,x)</summary>
		public static Vec4 _WWX(this Vec4 value) => new Vec4(0, value.w, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (0,w,w,y)</summary>
		public static Vec4 _WWY(this Vec4 value) => new Vec4(0, value.w, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (0,w,w,z)</summary>
		public static Vec4 _WWZ(this Vec4 value) => new Vec4(0, value.w, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (0,w,w,w)</summary>
		public static Vec4 _WWW(this Vec4 value) => new Vec4(0, value.w, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (x,0,0,0)</summary>
		public static Vec4 X___(this Vec4 value) => new Vec4(value.x, 0, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,0,0,x)</summary>
		public static Vec4 X__X(this Vec4 value) => new Vec4(value.x, 0, 0, value.x);

		/// <summary>A transpose swizzle property, returns (x,0,0,y)</summary>
		public static Vec4 X__Y(this Vec4 value) => new Vec4(value.x, 0, 0, value.y);

		/// <summary>A transpose swizzle property, returns (x,0,0,z)</summary>
		public static Vec4 X__Z(this Vec4 value) => new Vec4(value.x, 0, 0, value.z);

		/// <summary>A transpose swizzle property, returns (x,0,0,w)</summary>
		public static Vec4 X__W(this Vec4 value) => new Vec4(value.x, 0, 0, value.w);

		/// <summary>A transpose swizzle property, returns (x,0,x,0)</summary>
		public static Vec4 X_X_(this Vec4 value) => new Vec4(value.x, 0, value.x, 0);

		/// <summary>A transpose swizzle property, returns (x,0,x,x)</summary>
		public static Vec4 X_XX(this Vec4 value) => new Vec4(value.x, 0, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (x,0,x,y)</summary>
		public static Vec4 X_XY(this Vec4 value) => new Vec4(value.x, 0, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (x,0,x,z)</summary>
		public static Vec4 X_XZ(this Vec4 value) => new Vec4(value.x, 0, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (x,0,x,w)</summary>
		public static Vec4 X_XW(this Vec4 value) => new Vec4(value.x, 0, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (x,0,y,0)</summary>
		public static Vec4 X_Y_(this Vec4 value) => new Vec4(value.x, 0, value.y, 0);

		/// <summary>A transpose swizzle property, returns (x,0,y,x)</summary>
		public static Vec4 X_YX(this Vec4 value) => new Vec4(value.x, 0, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (x,0,y,y)</summary>
		public static Vec4 X_YY(this Vec4 value) => new Vec4(value.x, 0, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (x,0,y,z)</summary>
		public static Vec4 X_YZ(this Vec4 value) => new Vec4(value.x, 0, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (x,0,y,w)</summary>
		public static Vec4 X_YW(this Vec4 value) => new Vec4(value.x, 0, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (x,0,z,0)</summary>
		public static Vec4 X_Z_(this Vec4 value) => new Vec4(value.x, 0, value.z, 0);

		/// <summary>A transpose swizzle property, returns (x,0,z,x)</summary>
		public static Vec4 X_ZX(this Vec4 value) => new Vec4(value.x, 0, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (x,0,z,y)</summary>
		public static Vec4 X_ZY(this Vec4 value) => new Vec4(value.x, 0, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (x,0,z,z)</summary>
		public static Vec4 X_ZZ(this Vec4 value) => new Vec4(value.x, 0, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (x,0,z,w)</summary>
		public static Vec4 X_ZW(this Vec4 value) => new Vec4(value.x, 0, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (x,0,w,0)</summary>
		public static Vec4 X_W_(this Vec4 value) => new Vec4(value.x, 0, value.w, 0);

		/// <summary>A transpose swizzle property, returns (x,0,w,x)</summary>
		public static Vec4 X_WX(this Vec4 value) => new Vec4(value.x, 0, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (x,0,w,y)</summary>
		public static Vec4 X_WY(this Vec4 value) => new Vec4(value.x, 0, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (x,0,w,z)</summary>
		public static Vec4 X_WZ(this Vec4 value) => new Vec4(value.x, 0, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (x,0,w,w)</summary>
		public static Vec4 X_WW(this Vec4 value) => new Vec4(value.x, 0, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (x,x,0,0)</summary>
		public static Vec4 XX__(this Vec4 value) => new Vec4(value.x, value.x, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,x,0,x)</summary>
		public static Vec4 XX_X(this Vec4 value) => new Vec4(value.x, value.x, 0, value.x);

		/// <summary>A transpose swizzle property, returns (x,x,0,y)</summary>
		public static Vec4 XX_Y(this Vec4 value) => new Vec4(value.x, value.x, 0, value.y);

		/// <summary>A transpose swizzle property, returns (x,x,0,z)</summary>
		public static Vec4 XX_Z(this Vec4 value) => new Vec4(value.x, value.x, 0, value.z);

		/// <summary>A transpose swizzle property, returns (x,x,0,w)</summary>
		public static Vec4 XX_W(this Vec4 value) => new Vec4(value.x, value.x, 0, value.w);

		/// <summary>A transpose swizzle property, returns (x,x,x,0)</summary>
		public static Vec4 XXX_(this Vec4 value) => new Vec4(value.x, value.x, value.x, 0);

		/// <summary>A transpose swizzle property, returns (x,x,x,x)</summary>
		public static Vec4 XXXX(this Vec4 value) => new Vec4(value.x, value.x, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (x,x,x,y)</summary>
		public static Vec4 XXXY(this Vec4 value) => new Vec4(value.x, value.x, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (x,x,x,z)</summary>
		public static Vec4 XXXZ(this Vec4 value) => new Vec4(value.x, value.x, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (x,x,x,w)</summary>
		public static Vec4 XXXW(this Vec4 value) => new Vec4(value.x, value.x, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (x,x,y,0)</summary>
		public static Vec4 XXY_(this Vec4 value) => new Vec4(value.x, value.x, value.y, 0);

		/// <summary>A transpose swizzle property, returns (x,x,y,x)</summary>
		public static Vec4 XXYX(this Vec4 value) => new Vec4(value.x, value.x, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (x,x,y,y)</summary>
		public static Vec4 XXYY(this Vec4 value) => new Vec4(value.x, value.x, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (x,x,y,z)</summary>
		public static Vec4 XXYZ(this Vec4 value) => new Vec4(value.x, value.x, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (x,x,y,w)</summary>
		public static Vec4 XXYW(this Vec4 value) => new Vec4(value.x, value.x, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (x,x,z,0)</summary>
		public static Vec4 XXZ_(this Vec4 value) => new Vec4(value.x, value.x, value.z, 0);

		/// <summary>A transpose swizzle property, returns (x,x,z,x)</summary>
		public static Vec4 XXZX(this Vec4 value) => new Vec4(value.x, value.x, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (x,x,z,y)</summary>
		public static Vec4 XXZY(this Vec4 value) => new Vec4(value.x, value.x, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (x,x,z,z)</summary>
		public static Vec4 XXZZ(this Vec4 value) => new Vec4(value.x, value.x, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (x,x,z,w)</summary>
		public static Vec4 XXZW(this Vec4 value) => new Vec4(value.x, value.x, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (x,x,w,0)</summary>
		public static Vec4 XXW_(this Vec4 value) => new Vec4(value.x, value.x, value.w, 0);

		/// <summary>A transpose swizzle property, returns (x,x,w,x)</summary>
		public static Vec4 XXWX(this Vec4 value) => new Vec4(value.x, value.x, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (x,x,w,y)</summary>
		public static Vec4 XXWY(this Vec4 value) => new Vec4(value.x, value.x, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (x,x,w,z)</summary>
		public static Vec4 XXWZ(this Vec4 value) => new Vec4(value.x, value.x, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (x,x,w,w)</summary>
		public static Vec4 XXWW(this Vec4 value) => new Vec4(value.x, value.x, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (x,y,0,0)</summary>
		public static Vec4 XY__(this Vec4 value) => new Vec4(value.x, value.y, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,y,0,x)</summary>
		public static Vec4 XY_X(this Vec4 value) => new Vec4(value.x, value.y, 0, value.x);

		/// <summary>A transpose swizzle property, returns (x,y,0,y)</summary>
		public static Vec4 XY_Y(this Vec4 value) => new Vec4(value.x, value.y, 0, value.y);

		/// <summary>A transpose swizzle property, returns (x,y,0,z)</summary>
		public static Vec4 XY_Z(this Vec4 value) => new Vec4(value.x, value.y, 0, value.z);

		/// <summary>A transpose swizzle property, returns (x,y,0,w)</summary>
		public static Vec4 XY_W(this Vec4 value) => new Vec4(value.x, value.y, 0, value.w);

		/// <summary>A transpose swizzle property, returns (x,y,x,0)</summary>
		public static Vec4 XYX_(this Vec4 value) => new Vec4(value.x, value.y, value.x, 0);

		/// <summary>A transpose swizzle property, returns (x,y,x,x)</summary>
		public static Vec4 XYXX(this Vec4 value) => new Vec4(value.x, value.y, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (x,y,x,y)</summary>
		public static Vec4 XYXY(this Vec4 value) => new Vec4(value.x, value.y, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (x,y,x,z)</summary>
		public static Vec4 XYXZ(this Vec4 value) => new Vec4(value.x, value.y, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (x,y,x,w)</summary>
		public static Vec4 XYXW(this Vec4 value) => new Vec4(value.x, value.y, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (x,y,y,0)</summary>
		public static Vec4 XYY_(this Vec4 value) => new Vec4(value.x, value.y, value.y, 0);

		/// <summary>A transpose swizzle property, returns (x,y,y,x)</summary>
		public static Vec4 XYYX(this Vec4 value) => new Vec4(value.x, value.y, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (x,y,y,y)</summary>
		public static Vec4 XYYY(this Vec4 value) => new Vec4(value.x, value.y, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (x,y,y,z)</summary>
		public static Vec4 XYYZ(this Vec4 value) => new Vec4(value.x, value.y, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (x,y,y,w)</summary>
		public static Vec4 XYYW(this Vec4 value) => new Vec4(value.x, value.y, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (x,y,z,0)</summary>
		public static Vec4 XYZ_(this Vec4 value) => new Vec4(value.x, value.y, value.z, 0);

		/// <summary>A transpose swizzle property, returns (x,y,z,x)</summary>
		public static Vec4 XYZX(this Vec4 value) => new Vec4(value.x, value.y, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (x,y,z,y)</summary>
		public static Vec4 XYZY(this Vec4 value) => new Vec4(value.x, value.y, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (x,y,z,z)</summary>
		public static Vec4 XYZZ(this Vec4 value) => new Vec4(value.x, value.y, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (x,y,z,w)</summary>
		public static Vec4 XYZW(this Vec4 value) => new Vec4(value.x, value.y, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (x,y,w,0)</summary>
		public static Vec4 XYW_(this Vec4 value) => new Vec4(value.x, value.y, value.w, 0);

		/// <summary>A transpose swizzle property, returns (x,y,w,x)</summary>
		public static Vec4 XYWX(this Vec4 value) => new Vec4(value.x, value.y, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (x,y,w,y)</summary>
		public static Vec4 XYWY(this Vec4 value) => new Vec4(value.x, value.y, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (x,y,w,z)</summary>
		public static Vec4 XYWZ(this Vec4 value) => new Vec4(value.x, value.y, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (x,y,w,w)</summary>
		public static Vec4 XYWW(this Vec4 value) => new Vec4(value.x, value.y, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (x,z,0,0)</summary>
		public static Vec4 XZ__(this Vec4 value) => new Vec4(value.x, value.z, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,z,0,x)</summary>
		public static Vec4 XZ_X(this Vec4 value) => new Vec4(value.x, value.z, 0, value.x);

		/// <summary>A transpose swizzle property, returns (x,z,0,y)</summary>
		public static Vec4 XZ_Y(this Vec4 value) => new Vec4(value.x, value.z, 0, value.y);

		/// <summary>A transpose swizzle property, returns (x,z,0,z)</summary>
		public static Vec4 XZ_Z(this Vec4 value) => new Vec4(value.x, value.z, 0, value.z);

		/// <summary>A transpose swizzle property, returns (x,z,0,w)</summary>
		public static Vec4 XZ_W(this Vec4 value) => new Vec4(value.x, value.z, 0, value.w);

		/// <summary>A transpose swizzle property, returns (x,z,x,0)</summary>
		public static Vec4 XZX_(this Vec4 value) => new Vec4(value.x, value.z, value.x, 0);

		/// <summary>A transpose swizzle property, returns (x,z,x,x)</summary>
		public static Vec4 XZXX(this Vec4 value) => new Vec4(value.x, value.z, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (x,z,x,y)</summary>
		public static Vec4 XZXY(this Vec4 value) => new Vec4(value.x, value.z, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (x,z,x,z)</summary>
		public static Vec4 XZXZ(this Vec4 value) => new Vec4(value.x, value.z, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (x,z,x,w)</summary>
		public static Vec4 XZXW(this Vec4 value) => new Vec4(value.x, value.z, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (x,z,y,0)</summary>
		public static Vec4 XZY_(this Vec4 value) => new Vec4(value.x, value.z, value.y, 0);

		/// <summary>A transpose swizzle property, returns (x,z,y,x)</summary>
		public static Vec4 XZYX(this Vec4 value) => new Vec4(value.x, value.z, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (x,z,y,y)</summary>
		public static Vec4 XZYY(this Vec4 value) => new Vec4(value.x, value.z, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (x,z,y,z)</summary>
		public static Vec4 XZYZ(this Vec4 value) => new Vec4(value.x, value.z, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (x,z,y,w)</summary>
		public static Vec4 XZYW(this Vec4 value) => new Vec4(value.x, value.z, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (x,z,z,0)</summary>
		public static Vec4 XZZ_(this Vec4 value) => new Vec4(value.x, value.z, value.z, 0);

		/// <summary>A transpose swizzle property, returns (x,z,z,x)</summary>
		public static Vec4 XZZX(this Vec4 value) => new Vec4(value.x, value.z, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (x,z,z,y)</summary>
		public static Vec4 XZZY(this Vec4 value) => new Vec4(value.x, value.z, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (x,z,z,z)</summary>
		public static Vec4 XZZZ(this Vec4 value) => new Vec4(value.x, value.z, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (x,z,z,w)</summary>
		public static Vec4 XZZW(this Vec4 value) => new Vec4(value.x, value.z, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (x,z,w,0)</summary>
		public static Vec4 XZW_(this Vec4 value) => new Vec4(value.x, value.z, value.w, 0);

		/// <summary>A transpose swizzle property, returns (x,z,w,x)</summary>
		public static Vec4 XZWX(this Vec4 value) => new Vec4(value.x, value.z, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (x,z,w,y)</summary>
		public static Vec4 XZWY(this Vec4 value) => new Vec4(value.x, value.z, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (x,z,w,z)</summary>
		public static Vec4 XZWZ(this Vec4 value) => new Vec4(value.x, value.z, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (x,z,w,w)</summary>
		public static Vec4 XZWW(this Vec4 value) => new Vec4(value.x, value.z, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (x,w,0,0)</summary>
		public static Vec4 XW__(this Vec4 value) => new Vec4(value.x, value.w, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,w,0,x)</summary>
		public static Vec4 XW_X(this Vec4 value) => new Vec4(value.x, value.w, 0, value.x);

		/// <summary>A transpose swizzle property, returns (x,w,0,y)</summary>
		public static Vec4 XW_Y(this Vec4 value) => new Vec4(value.x, value.w, 0, value.y);

		/// <summary>A transpose swizzle property, returns (x,w,0,z)</summary>
		public static Vec4 XW_Z(this Vec4 value) => new Vec4(value.x, value.w, 0, value.z);

		/// <summary>A transpose swizzle property, returns (x,w,0,w)</summary>
		public static Vec4 XW_W(this Vec4 value) => new Vec4(value.x, value.w, 0, value.w);

		/// <summary>A transpose swizzle property, returns (x,w,x,0)</summary>
		public static Vec4 XWX_(this Vec4 value) => new Vec4(value.x, value.w, value.x, 0);

		/// <summary>A transpose swizzle property, returns (x,w,x,x)</summary>
		public static Vec4 XWXX(this Vec4 value) => new Vec4(value.x, value.w, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (x,w,x,y)</summary>
		public static Vec4 XWXY(this Vec4 value) => new Vec4(value.x, value.w, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (x,w,x,z)</summary>
		public static Vec4 XWXZ(this Vec4 value) => new Vec4(value.x, value.w, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (x,w,x,w)</summary>
		public static Vec4 XWXW(this Vec4 value) => new Vec4(value.x, value.w, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (x,w,y,0)</summary>
		public static Vec4 XWY_(this Vec4 value) => new Vec4(value.x, value.w, value.y, 0);

		/// <summary>A transpose swizzle property, returns (x,w,y,x)</summary>
		public static Vec4 XWYX(this Vec4 value) => new Vec4(value.x, value.w, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (x,w,y,y)</summary>
		public static Vec4 XWYY(this Vec4 value) => new Vec4(value.x, value.w, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (x,w,y,z)</summary>
		public static Vec4 XWYZ(this Vec4 value) => new Vec4(value.x, value.w, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (x,w,y,w)</summary>
		public static Vec4 XWYW(this Vec4 value) => new Vec4(value.x, value.w, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (x,w,z,0)</summary>
		public static Vec4 XWZ_(this Vec4 value) => new Vec4(value.x, value.w, value.z, 0);

		/// <summary>A transpose swizzle property, returns (x,w,z,x)</summary>
		public static Vec4 XWZX(this Vec4 value) => new Vec4(value.x, value.w, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (x,w,z,y)</summary>
		public static Vec4 XWZY(this Vec4 value) => new Vec4(value.x, value.w, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (x,w,z,z)</summary>
		public static Vec4 XWZZ(this Vec4 value) => new Vec4(value.x, value.w, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (x,w,z,w)</summary>
		public static Vec4 XWZW(this Vec4 value) => new Vec4(value.x, value.w, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (x,w,w,0)</summary>
		public static Vec4 XWW_(this Vec4 value) => new Vec4(value.x, value.w, value.w, 0);

		/// <summary>A transpose swizzle property, returns (x,w,w,x)</summary>
		public static Vec4 XWWX(this Vec4 value) => new Vec4(value.x, value.w, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (x,w,w,y)</summary>
		public static Vec4 XWWY(this Vec4 value) => new Vec4(value.x, value.w, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (x,w,w,z)</summary>
		public static Vec4 XWWZ(this Vec4 value) => new Vec4(value.x, value.w, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (x,w,w,w)</summary>
		public static Vec4 XWWW(this Vec4 value) => new Vec4(value.x, value.w, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (y,0,0,0)</summary>
		public static Vec4 Y___(this Vec4 value) => new Vec4(value.y, 0, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,0,0,x)</summary>
		public static Vec4 Y__X(this Vec4 value) => new Vec4(value.y, 0, 0, value.x);

		/// <summary>A transpose swizzle property, returns (y,0,0,y)</summary>
		public static Vec4 Y__Y(this Vec4 value) => new Vec4(value.y, 0, 0, value.y);

		/// <summary>A transpose swizzle property, returns (y,0,0,z)</summary>
		public static Vec4 Y__Z(this Vec4 value) => new Vec4(value.y, 0, 0, value.z);

		/// <summary>A transpose swizzle property, returns (y,0,0,w)</summary>
		public static Vec4 Y__W(this Vec4 value) => new Vec4(value.y, 0, 0, value.w);

		/// <summary>A transpose swizzle property, returns (y,0,x,0)</summary>
		public static Vec4 Y_X_(this Vec4 value) => new Vec4(value.y, 0, value.x, 0);

		/// <summary>A transpose swizzle property, returns (y,0,x,x)</summary>
		public static Vec4 Y_XX(this Vec4 value) => new Vec4(value.y, 0, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (y,0,x,y)</summary>
		public static Vec4 Y_XY(this Vec4 value) => new Vec4(value.y, 0, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (y,0,x,z)</summary>
		public static Vec4 Y_XZ(this Vec4 value) => new Vec4(value.y, 0, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (y,0,x,w)</summary>
		public static Vec4 Y_XW(this Vec4 value) => new Vec4(value.y, 0, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (y,0,y,0)</summary>
		public static Vec4 Y_Y_(this Vec4 value) => new Vec4(value.y, 0, value.y, 0);

		/// <summary>A transpose swizzle property, returns (y,0,y,x)</summary>
		public static Vec4 Y_YX(this Vec4 value) => new Vec4(value.y, 0, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (y,0,y,y)</summary>
		public static Vec4 Y_YY(this Vec4 value) => new Vec4(value.y, 0, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (y,0,y,z)</summary>
		public static Vec4 Y_YZ(this Vec4 value) => new Vec4(value.y, 0, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (y,0,y,w)</summary>
		public static Vec4 Y_YW(this Vec4 value) => new Vec4(value.y, 0, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (y,0,z,0)</summary>
		public static Vec4 Y_Z_(this Vec4 value) => new Vec4(value.y, 0, value.z, 0);

		/// <summary>A transpose swizzle property, returns (y,0,z,x)</summary>
		public static Vec4 Y_ZX(this Vec4 value) => new Vec4(value.y, 0, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (y,0,z,y)</summary>
		public static Vec4 Y_ZY(this Vec4 value) => new Vec4(value.y, 0, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (y,0,z,z)</summary>
		public static Vec4 Y_ZZ(this Vec4 value) => new Vec4(value.y, 0, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (y,0,z,w)</summary>
		public static Vec4 Y_ZW(this Vec4 value) => new Vec4(value.y, 0, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (y,0,w,0)</summary>
		public static Vec4 Y_W_(this Vec4 value) => new Vec4(value.y, 0, value.w, 0);

		/// <summary>A transpose swizzle property, returns (y,0,w,x)</summary>
		public static Vec4 Y_WX(this Vec4 value) => new Vec4(value.y, 0, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (y,0,w,y)</summary>
		public static Vec4 Y_WY(this Vec4 value) => new Vec4(value.y, 0, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (y,0,w,z)</summary>
		public static Vec4 Y_WZ(this Vec4 value) => new Vec4(value.y, 0, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (y,0,w,w)</summary>
		public static Vec4 Y_WW(this Vec4 value) => new Vec4(value.y, 0, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (y,x,0,0)</summary>
		public static Vec4 YX__(this Vec4 value) => new Vec4(value.y, value.x, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,x,0,x)</summary>
		public static Vec4 YX_X(this Vec4 value) => new Vec4(value.y, value.x, 0, value.x);

		/// <summary>A transpose swizzle property, returns (y,x,0,y)</summary>
		public static Vec4 YX_Y(this Vec4 value) => new Vec4(value.y, value.x, 0, value.y);

		/// <summary>A transpose swizzle property, returns (y,x,0,z)</summary>
		public static Vec4 YX_Z(this Vec4 value) => new Vec4(value.y, value.x, 0, value.z);

		/// <summary>A transpose swizzle property, returns (y,x,0,w)</summary>
		public static Vec4 YX_W(this Vec4 value) => new Vec4(value.y, value.x, 0, value.w);

		/// <summary>A transpose swizzle property, returns (y,x,x,0)</summary>
		public static Vec4 YXX_(this Vec4 value) => new Vec4(value.y, value.x, value.x, 0);

		/// <summary>A transpose swizzle property, returns (y,x,x,x)</summary>
		public static Vec4 YXXX(this Vec4 value) => new Vec4(value.y, value.x, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (y,x,x,y)</summary>
		public static Vec4 YXXY(this Vec4 value) => new Vec4(value.y, value.x, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (y,x,x,z)</summary>
		public static Vec4 YXXZ(this Vec4 value) => new Vec4(value.y, value.x, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (y,x,x,w)</summary>
		public static Vec4 YXXW(this Vec4 value) => new Vec4(value.y, value.x, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (y,x,y,0)</summary>
		public static Vec4 YXY_(this Vec4 value) => new Vec4(value.y, value.x, value.y, 0);

		/// <summary>A transpose swizzle property, returns (y,x,y,x)</summary>
		public static Vec4 YXYX(this Vec4 value) => new Vec4(value.y, value.x, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (y,x,y,y)</summary>
		public static Vec4 YXYY(this Vec4 value) => new Vec4(value.y, value.x, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (y,x,y,z)</summary>
		public static Vec4 YXYZ(this Vec4 value) => new Vec4(value.y, value.x, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (y,x,y,w)</summary>
		public static Vec4 YXYW(this Vec4 value) => new Vec4(value.y, value.x, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (y,x,z,0)</summary>
		public static Vec4 YXZ_(this Vec4 value) => new Vec4(value.y, value.x, value.z, 0);

		/// <summary>A transpose swizzle property, returns (y,x,z,x)</summary>
		public static Vec4 YXZX(this Vec4 value) => new Vec4(value.y, value.x, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (y,x,z,y)</summary>
		public static Vec4 YXZY(this Vec4 value) => new Vec4(value.y, value.x, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (y,x,z,z)</summary>
		public static Vec4 YXZZ(this Vec4 value) => new Vec4(value.y, value.x, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (y,x,z,w)</summary>
		public static Vec4 YXZW(this Vec4 value) => new Vec4(value.y, value.x, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (y,x,w,0)</summary>
		public static Vec4 YXW_(this Vec4 value) => new Vec4(value.y, value.x, value.w, 0);

		/// <summary>A transpose swizzle property, returns (y,x,w,x)</summary>
		public static Vec4 YXWX(this Vec4 value) => new Vec4(value.y, value.x, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (y,x,w,y)</summary>
		public static Vec4 YXWY(this Vec4 value) => new Vec4(value.y, value.x, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (y,x,w,z)</summary>
		public static Vec4 YXWZ(this Vec4 value) => new Vec4(value.y, value.x, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (y,x,w,w)</summary>
		public static Vec4 YXWW(this Vec4 value) => new Vec4(value.y, value.x, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (y,y,0,0)</summary>
		public static Vec4 YY__(this Vec4 value) => new Vec4(value.y, value.y, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,y,0,x)</summary>
		public static Vec4 YY_X(this Vec4 value) => new Vec4(value.y, value.y, 0, value.x);

		/// <summary>A transpose swizzle property, returns (y,y,0,y)</summary>
		public static Vec4 YY_Y(this Vec4 value) => new Vec4(value.y, value.y, 0, value.y);

		/// <summary>A transpose swizzle property, returns (y,y,0,z)</summary>
		public static Vec4 YY_Z(this Vec4 value) => new Vec4(value.y, value.y, 0, value.z);

		/// <summary>A transpose swizzle property, returns (y,y,0,w)</summary>
		public static Vec4 YY_W(this Vec4 value) => new Vec4(value.y, value.y, 0, value.w);

		/// <summary>A transpose swizzle property, returns (y,y,x,0)</summary>
		public static Vec4 YYX_(this Vec4 value) => new Vec4(value.y, value.y, value.x, 0);

		/// <summary>A transpose swizzle property, returns (y,y,x,x)</summary>
		public static Vec4 YYXX(this Vec4 value) => new Vec4(value.y, value.y, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (y,y,x,y)</summary>
		public static Vec4 YYXY(this Vec4 value) => new Vec4(value.y, value.y, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (y,y,x,z)</summary>
		public static Vec4 YYXZ(this Vec4 value) => new Vec4(value.y, value.y, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (y,y,x,w)</summary>
		public static Vec4 YYXW(this Vec4 value) => new Vec4(value.y, value.y, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (y,y,y,0)</summary>
		public static Vec4 YYY_(this Vec4 value) => new Vec4(value.y, value.y, value.y, 0);

		/// <summary>A transpose swizzle property, returns (y,y,y,x)</summary>
		public static Vec4 YYYX(this Vec4 value) => new Vec4(value.y, value.y, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (y,y,y,y)</summary>
		public static Vec4 YYYY(this Vec4 value) => new Vec4(value.y, value.y, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (y,y,y,z)</summary>
		public static Vec4 YYYZ(this Vec4 value) => new Vec4(value.y, value.y, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (y,y,y,w)</summary>
		public static Vec4 YYYW(this Vec4 value) => new Vec4(value.y, value.y, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (y,y,z,0)</summary>
		public static Vec4 YYZ_(this Vec4 value) => new Vec4(value.y, value.y, value.z, 0);

		/// <summary>A transpose swizzle property, returns (y,y,z,x)</summary>
		public static Vec4 YYZX(this Vec4 value) => new Vec4(value.y, value.y, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (y,y,z,y)</summary>
		public static Vec4 YYZY(this Vec4 value) => new Vec4(value.y, value.y, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (y,y,z,z)</summary>
		public static Vec4 YYZZ(this Vec4 value) => new Vec4(value.y, value.y, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (y,y,z,w)</summary>
		public static Vec4 YYZW(this Vec4 value) => new Vec4(value.y, value.y, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (y,y,w,0)</summary>
		public static Vec4 YYW_(this Vec4 value) => new Vec4(value.y, value.y, value.w, 0);

		/// <summary>A transpose swizzle property, returns (y,y,w,x)</summary>
		public static Vec4 YYWX(this Vec4 value) => new Vec4(value.y, value.y, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (y,y,w,y)</summary>
		public static Vec4 YYWY(this Vec4 value) => new Vec4(value.y, value.y, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (y,y,w,z)</summary>
		public static Vec4 YYWZ(this Vec4 value) => new Vec4(value.y, value.y, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (y,y,w,w)</summary>
		public static Vec4 YYWW(this Vec4 value) => new Vec4(value.y, value.y, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (y,z,0,0)</summary>
		public static Vec4 YZ__(this Vec4 value) => new Vec4(value.y, value.z, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,z,0,x)</summary>
		public static Vec4 YZ_X(this Vec4 value) => new Vec4(value.y, value.z, 0, value.x);

		/// <summary>A transpose swizzle property, returns (y,z,0,y)</summary>
		public static Vec4 YZ_Y(this Vec4 value) => new Vec4(value.y, value.z, 0, value.y);

		/// <summary>A transpose swizzle property, returns (y,z,0,z)</summary>
		public static Vec4 YZ_Z(this Vec4 value) => new Vec4(value.y, value.z, 0, value.z);

		/// <summary>A transpose swizzle property, returns (y,z,0,w)</summary>
		public static Vec4 YZ_W(this Vec4 value) => new Vec4(value.y, value.z, 0, value.w);

		/// <summary>A transpose swizzle property, returns (y,z,x,0)</summary>
		public static Vec4 YZX_(this Vec4 value) => new Vec4(value.y, value.z, value.x, 0);

		/// <summary>A transpose swizzle property, returns (y,z,x,x)</summary>
		public static Vec4 YZXX(this Vec4 value) => new Vec4(value.y, value.z, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (y,z,x,y)</summary>
		public static Vec4 YZXY(this Vec4 value) => new Vec4(value.y, value.z, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (y,z,x,z)</summary>
		public static Vec4 YZXZ(this Vec4 value) => new Vec4(value.y, value.z, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (y,z,x,w)</summary>
		public static Vec4 YZXW(this Vec4 value) => new Vec4(value.y, value.z, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (y,z,y,0)</summary>
		public static Vec4 YZY_(this Vec4 value) => new Vec4(value.y, value.z, value.y, 0);

		/// <summary>A transpose swizzle property, returns (y,z,y,x)</summary>
		public static Vec4 YZYX(this Vec4 value) => new Vec4(value.y, value.z, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (y,z,y,y)</summary>
		public static Vec4 YZYY(this Vec4 value) => new Vec4(value.y, value.z, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (y,z,y,z)</summary>
		public static Vec4 YZYZ(this Vec4 value) => new Vec4(value.y, value.z, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (y,z,y,w)</summary>
		public static Vec4 YZYW(this Vec4 value) => new Vec4(value.y, value.z, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (y,z,z,0)</summary>
		public static Vec4 YZZ_(this Vec4 value) => new Vec4(value.y, value.z, value.z, 0);

		/// <summary>A transpose swizzle property, returns (y,z,z,x)</summary>
		public static Vec4 YZZX(this Vec4 value) => new Vec4(value.y, value.z, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (y,z,z,y)</summary>
		public static Vec4 YZZY(this Vec4 value) => new Vec4(value.y, value.z, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (y,z,z,z)</summary>
		public static Vec4 YZZZ(this Vec4 value) => new Vec4(value.y, value.z, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (y,z,z,w)</summary>
		public static Vec4 YZZW(this Vec4 value) => new Vec4(value.y, value.z, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (y,z,w,0)</summary>
		public static Vec4 YZW_(this Vec4 value) => new Vec4(value.y, value.z, value.w, 0);

		/// <summary>A transpose swizzle property, returns (y,z,w,x)</summary>
		public static Vec4 YZWX(this Vec4 value) => new Vec4(value.y, value.z, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (y,z,w,y)</summary>
		public static Vec4 YZWY(this Vec4 value) => new Vec4(value.y, value.z, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (y,z,w,z)</summary>
		public static Vec4 YZWZ(this Vec4 value) => new Vec4(value.y, value.z, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (y,z,w,w)</summary>
		public static Vec4 YZWW(this Vec4 value) => new Vec4(value.y, value.z, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (y,w,0,0)</summary>
		public static Vec4 YW__(this Vec4 value) => new Vec4(value.y, value.w, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,w,0,x)</summary>
		public static Vec4 YW_X(this Vec4 value) => new Vec4(value.y, value.w, 0, value.x);

		/// <summary>A transpose swizzle property, returns (y,w,0,y)</summary>
		public static Vec4 YW_Y(this Vec4 value) => new Vec4(value.y, value.w, 0, value.y);

		/// <summary>A transpose swizzle property, returns (y,w,0,z)</summary>
		public static Vec4 YW_Z(this Vec4 value) => new Vec4(value.y, value.w, 0, value.z);

		/// <summary>A transpose swizzle property, returns (y,w,0,w)</summary>
		public static Vec4 YW_W(this Vec4 value) => new Vec4(value.y, value.w, 0, value.w);

		/// <summary>A transpose swizzle property, returns (y,w,x,0)</summary>
		public static Vec4 YWX_(this Vec4 value) => new Vec4(value.y, value.w, value.x, 0);

		/// <summary>A transpose swizzle property, returns (y,w,x,x)</summary>
		public static Vec4 YWXX(this Vec4 value) => new Vec4(value.y, value.w, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (y,w,x,y)</summary>
		public static Vec4 YWXY(this Vec4 value) => new Vec4(value.y, value.w, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (y,w,x,z)</summary>
		public static Vec4 YWXZ(this Vec4 value) => new Vec4(value.y, value.w, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (y,w,x,w)</summary>
		public static Vec4 YWXW(this Vec4 value) => new Vec4(value.y, value.w, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (y,w,y,0)</summary>
		public static Vec4 YWY_(this Vec4 value) => new Vec4(value.y, value.w, value.y, 0);

		/// <summary>A transpose swizzle property, returns (y,w,y,x)</summary>
		public static Vec4 YWYX(this Vec4 value) => new Vec4(value.y, value.w, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (y,w,y,y)</summary>
		public static Vec4 YWYY(this Vec4 value) => new Vec4(value.y, value.w, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (y,w,y,z)</summary>
		public static Vec4 YWYZ(this Vec4 value) => new Vec4(value.y, value.w, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (y,w,y,w)</summary>
		public static Vec4 YWYW(this Vec4 value) => new Vec4(value.y, value.w, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (y,w,z,0)</summary>
		public static Vec4 YWZ_(this Vec4 value) => new Vec4(value.y, value.w, value.z, 0);

		/// <summary>A transpose swizzle property, returns (y,w,z,x)</summary>
		public static Vec4 YWZX(this Vec4 value) => new Vec4(value.y, value.w, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (y,w,z,y)</summary>
		public static Vec4 YWZY(this Vec4 value) => new Vec4(value.y, value.w, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (y,w,z,z)</summary>
		public static Vec4 YWZZ(this Vec4 value) => new Vec4(value.y, value.w, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (y,w,z,w)</summary>
		public static Vec4 YWZW(this Vec4 value) => new Vec4(value.y, value.w, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (y,w,w,0)</summary>
		public static Vec4 YWW_(this Vec4 value) => new Vec4(value.y, value.w, value.w, 0);

		/// <summary>A transpose swizzle property, returns (y,w,w,x)</summary>
		public static Vec4 YWWX(this Vec4 value) => new Vec4(value.y, value.w, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (y,w,w,y)</summary>
		public static Vec4 YWWY(this Vec4 value) => new Vec4(value.y, value.w, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (y,w,w,z)</summary>
		public static Vec4 YWWZ(this Vec4 value) => new Vec4(value.y, value.w, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (y,w,w,w)</summary>
		public static Vec4 YWWW(this Vec4 value) => new Vec4(value.y, value.w, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (z,0,0,0)</summary>
		public static Vec4 Z___(this Vec4 value) => new Vec4(value.z, 0, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,0,0,x)</summary>
		public static Vec4 Z__X(this Vec4 value) => new Vec4(value.z, 0, 0, value.x);

		/// <summary>A transpose swizzle property, returns (z,0,0,y)</summary>
		public static Vec4 Z__Y(this Vec4 value) => new Vec4(value.z, 0, 0, value.y);

		/// <summary>A transpose swizzle property, returns (z,0,0,z)</summary>
		public static Vec4 Z__Z(this Vec4 value) => new Vec4(value.z, 0, 0, value.z);

		/// <summary>A transpose swizzle property, returns (z,0,0,w)</summary>
		public static Vec4 Z__W(this Vec4 value) => new Vec4(value.z, 0, 0, value.w);

		/// <summary>A transpose swizzle property, returns (z,0,x,0)</summary>
		public static Vec4 Z_X_(this Vec4 value) => new Vec4(value.z, 0, value.x, 0);

		/// <summary>A transpose swizzle property, returns (z,0,x,x)</summary>
		public static Vec4 Z_XX(this Vec4 value) => new Vec4(value.z, 0, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (z,0,x,y)</summary>
		public static Vec4 Z_XY(this Vec4 value) => new Vec4(value.z, 0, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (z,0,x,z)</summary>
		public static Vec4 Z_XZ(this Vec4 value) => new Vec4(value.z, 0, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (z,0,x,w)</summary>
		public static Vec4 Z_XW(this Vec4 value) => new Vec4(value.z, 0, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (z,0,y,0)</summary>
		public static Vec4 Z_Y_(this Vec4 value) => new Vec4(value.z, 0, value.y, 0);

		/// <summary>A transpose swizzle property, returns (z,0,y,x)</summary>
		public static Vec4 Z_YX(this Vec4 value) => new Vec4(value.z, 0, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (z,0,y,y)</summary>
		public static Vec4 Z_YY(this Vec4 value) => new Vec4(value.z, 0, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (z,0,y,z)</summary>
		public static Vec4 Z_YZ(this Vec4 value) => new Vec4(value.z, 0, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (z,0,y,w)</summary>
		public static Vec4 Z_YW(this Vec4 value) => new Vec4(value.z, 0, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (z,0,z,0)</summary>
		public static Vec4 Z_Z_(this Vec4 value) => new Vec4(value.z, 0, value.z, 0);

		/// <summary>A transpose swizzle property, returns (z,0,z,x)</summary>
		public static Vec4 Z_ZX(this Vec4 value) => new Vec4(value.z, 0, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (z,0,z,y)</summary>
		public static Vec4 Z_ZY(this Vec4 value) => new Vec4(value.z, 0, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (z,0,z,z)</summary>
		public static Vec4 Z_ZZ(this Vec4 value) => new Vec4(value.z, 0, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (z,0,z,w)</summary>
		public static Vec4 Z_ZW(this Vec4 value) => new Vec4(value.z, 0, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (z,0,w,0)</summary>
		public static Vec4 Z_W_(this Vec4 value) => new Vec4(value.z, 0, value.w, 0);

		/// <summary>A transpose swizzle property, returns (z,0,w,x)</summary>
		public static Vec4 Z_WX(this Vec4 value) => new Vec4(value.z, 0, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (z,0,w,y)</summary>
		public static Vec4 Z_WY(this Vec4 value) => new Vec4(value.z, 0, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (z,0,w,z)</summary>
		public static Vec4 Z_WZ(this Vec4 value) => new Vec4(value.z, 0, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (z,0,w,w)</summary>
		public static Vec4 Z_WW(this Vec4 value) => new Vec4(value.z, 0, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (z,x,0,0)</summary>
		public static Vec4 ZX__(this Vec4 value) => new Vec4(value.z, value.x, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,x,0,x)</summary>
		public static Vec4 ZX_X(this Vec4 value) => new Vec4(value.z, value.x, 0, value.x);

		/// <summary>A transpose swizzle property, returns (z,x,0,y)</summary>
		public static Vec4 ZX_Y(this Vec4 value) => new Vec4(value.z, value.x, 0, value.y);

		/// <summary>A transpose swizzle property, returns (z,x,0,z)</summary>
		public static Vec4 ZX_Z(this Vec4 value) => new Vec4(value.z, value.x, 0, value.z);

		/// <summary>A transpose swizzle property, returns (z,x,0,w)</summary>
		public static Vec4 ZX_W(this Vec4 value) => new Vec4(value.z, value.x, 0, value.w);

		/// <summary>A transpose swizzle property, returns (z,x,x,0)</summary>
		public static Vec4 ZXX_(this Vec4 value) => new Vec4(value.z, value.x, value.x, 0);

		/// <summary>A transpose swizzle property, returns (z,x,x,x)</summary>
		public static Vec4 ZXXX(this Vec4 value) => new Vec4(value.z, value.x, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (z,x,x,y)</summary>
		public static Vec4 ZXXY(this Vec4 value) => new Vec4(value.z, value.x, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (z,x,x,z)</summary>
		public static Vec4 ZXXZ(this Vec4 value) => new Vec4(value.z, value.x, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (z,x,x,w)</summary>
		public static Vec4 ZXXW(this Vec4 value) => new Vec4(value.z, value.x, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (z,x,y,0)</summary>
		public static Vec4 ZXY_(this Vec4 value) => new Vec4(value.z, value.x, value.y, 0);

		/// <summary>A transpose swizzle property, returns (z,x,y,x)</summary>
		public static Vec4 ZXYX(this Vec4 value) => new Vec4(value.z, value.x, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (z,x,y,y)</summary>
		public static Vec4 ZXYY(this Vec4 value) => new Vec4(value.z, value.x, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (z,x,y,z)</summary>
		public static Vec4 ZXYZ(this Vec4 value) => new Vec4(value.z, value.x, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (z,x,y,w)</summary>
		public static Vec4 ZXYW(this Vec4 value) => new Vec4(value.z, value.x, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (z,x,z,0)</summary>
		public static Vec4 ZXZ_(this Vec4 value) => new Vec4(value.z, value.x, value.z, 0);

		/// <summary>A transpose swizzle property, returns (z,x,z,x)</summary>
		public static Vec4 ZXZX(this Vec4 value) => new Vec4(value.z, value.x, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (z,x,z,y)</summary>
		public static Vec4 ZXZY(this Vec4 value) => new Vec4(value.z, value.x, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (z,x,z,z)</summary>
		public static Vec4 ZXZZ(this Vec4 value) => new Vec4(value.z, value.x, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (z,x,z,w)</summary>
		public static Vec4 ZXZW(this Vec4 value) => new Vec4(value.z, value.x, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (z,x,w,0)</summary>
		public static Vec4 ZXW_(this Vec4 value) => new Vec4(value.z, value.x, value.w, 0);

		/// <summary>A transpose swizzle property, returns (z,x,w,x)</summary>
		public static Vec4 ZXWX(this Vec4 value) => new Vec4(value.z, value.x, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (z,x,w,y)</summary>
		public static Vec4 ZXWY(this Vec4 value) => new Vec4(value.z, value.x, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (z,x,w,z)</summary>
		public static Vec4 ZXWZ(this Vec4 value) => new Vec4(value.z, value.x, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (z,x,w,w)</summary>
		public static Vec4 ZXWW(this Vec4 value) => new Vec4(value.z, value.x, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (z,y,0,0)</summary>
		public static Vec4 ZY__(this Vec4 value) => new Vec4(value.z, value.y, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,y,0,x)</summary>
		public static Vec4 ZY_X(this Vec4 value) => new Vec4(value.z, value.y, 0, value.x);

		/// <summary>A transpose swizzle property, returns (z,y,0,y)</summary>
		public static Vec4 ZY_Y(this Vec4 value) => new Vec4(value.z, value.y, 0, value.y);

		/// <summary>A transpose swizzle property, returns (z,y,0,z)</summary>
		public static Vec4 ZY_Z(this Vec4 value) => new Vec4(value.z, value.y, 0, value.z);

		/// <summary>A transpose swizzle property, returns (z,y,0,w)</summary>
		public static Vec4 ZY_W(this Vec4 value) => new Vec4(value.z, value.y, 0, value.w);

		/// <summary>A transpose swizzle property, returns (z,y,x,0)</summary>
		public static Vec4 ZYX_(this Vec4 value) => new Vec4(value.z, value.y, value.x, 0);

		/// <summary>A transpose swizzle property, returns (z,y,x,x)</summary>
		public static Vec4 ZYXX(this Vec4 value) => new Vec4(value.z, value.y, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (z,y,x,y)</summary>
		public static Vec4 ZYXY(this Vec4 value) => new Vec4(value.z, value.y, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (z,y,x,z)</summary>
		public static Vec4 ZYXZ(this Vec4 value) => new Vec4(value.z, value.y, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (z,y,x,w)</summary>
		public static Vec4 ZYXW(this Vec4 value) => new Vec4(value.z, value.y, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (z,y,y,0)</summary>
		public static Vec4 ZYY_(this Vec4 value) => new Vec4(value.z, value.y, value.y, 0);

		/// <summary>A transpose swizzle property, returns (z,y,y,x)</summary>
		public static Vec4 ZYYX(this Vec4 value) => new Vec4(value.z, value.y, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (z,y,y,y)</summary>
		public static Vec4 ZYYY(this Vec4 value) => new Vec4(value.z, value.y, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (z,y,y,z)</summary>
		public static Vec4 ZYYZ(this Vec4 value) => new Vec4(value.z, value.y, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (z,y,y,w)</summary>
		public static Vec4 ZYYW(this Vec4 value) => new Vec4(value.z, value.y, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (z,y,z,0)</summary>
		public static Vec4 ZYZ_(this Vec4 value) => new Vec4(value.z, value.y, value.z, 0);

		/// <summary>A transpose swizzle property, returns (z,y,z,x)</summary>
		public static Vec4 ZYZX(this Vec4 value) => new Vec4(value.z, value.y, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (z,y,z,y)</summary>
		public static Vec4 ZYZY(this Vec4 value) => new Vec4(value.z, value.y, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (z,y,z,z)</summary>
		public static Vec4 ZYZZ(this Vec4 value) => new Vec4(value.z, value.y, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (z,y,z,w)</summary>
		public static Vec4 ZYZW(this Vec4 value) => new Vec4(value.z, value.y, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (z,y,w,0)</summary>
		public static Vec4 ZYW_(this Vec4 value) => new Vec4(value.z, value.y, value.w, 0);

		/// <summary>A transpose swizzle property, returns (z,y,w,x)</summary>
		public static Vec4 ZYWX(this Vec4 value) => new Vec4(value.z, value.y, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (z,y,w,y)</summary>
		public static Vec4 ZYWY(this Vec4 value) => new Vec4(value.z, value.y, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (z,y,w,z)</summary>
		public static Vec4 ZYWZ(this Vec4 value) => new Vec4(value.z, value.y, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (z,y,w,w)</summary>
		public static Vec4 ZYWW(this Vec4 value) => new Vec4(value.z, value.y, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (z,z,0,0)</summary>
		public static Vec4 ZZ__(this Vec4 value) => new Vec4(value.z, value.z, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,z,0,x)</summary>
		public static Vec4 ZZ_X(this Vec4 value) => new Vec4(value.z, value.z, 0, value.x);

		/// <summary>A transpose swizzle property, returns (z,z,0,y)</summary>
		public static Vec4 ZZ_Y(this Vec4 value) => new Vec4(value.z, value.z, 0, value.y);

		/// <summary>A transpose swizzle property, returns (z,z,0,z)</summary>
		public static Vec4 ZZ_Z(this Vec4 value) => new Vec4(value.z, value.z, 0, value.z);

		/// <summary>A transpose swizzle property, returns (z,z,0,w)</summary>
		public static Vec4 ZZ_W(this Vec4 value) => new Vec4(value.z, value.z, 0, value.w);

		/// <summary>A transpose swizzle property, returns (z,z,x,0)</summary>
		public static Vec4 ZZX_(this Vec4 value) => new Vec4(value.z, value.z, value.x, 0);

		/// <summary>A transpose swizzle property, returns (z,z,x,x)</summary>
		public static Vec4 ZZXX(this Vec4 value) => new Vec4(value.z, value.z, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (z,z,x,y)</summary>
		public static Vec4 ZZXY(this Vec4 value) => new Vec4(value.z, value.z, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (z,z,x,z)</summary>
		public static Vec4 ZZXZ(this Vec4 value) => new Vec4(value.z, value.z, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (z,z,x,w)</summary>
		public static Vec4 ZZXW(this Vec4 value) => new Vec4(value.z, value.z, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (z,z,y,0)</summary>
		public static Vec4 ZZY_(this Vec4 value) => new Vec4(value.z, value.z, value.y, 0);

		/// <summary>A transpose swizzle property, returns (z,z,y,x)</summary>
		public static Vec4 ZZYX(this Vec4 value) => new Vec4(value.z, value.z, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (z,z,y,y)</summary>
		public static Vec4 ZZYY(this Vec4 value) => new Vec4(value.z, value.z, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (z,z,y,z)</summary>
		public static Vec4 ZZYZ(this Vec4 value) => new Vec4(value.z, value.z, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (z,z,y,w)</summary>
		public static Vec4 ZZYW(this Vec4 value) => new Vec4(value.z, value.z, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (z,z,z,0)</summary>
		public static Vec4 ZZZ_(this Vec4 value) => new Vec4(value.z, value.z, value.z, 0);

		/// <summary>A transpose swizzle property, returns (z,z,z,x)</summary>
		public static Vec4 ZZZX(this Vec4 value) => new Vec4(value.z, value.z, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (z,z,z,y)</summary>
		public static Vec4 ZZZY(this Vec4 value) => new Vec4(value.z, value.z, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (z,z,z,z)</summary>
		public static Vec4 ZZZZ(this Vec4 value) => new Vec4(value.z, value.z, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (z,z,z,w)</summary>
		public static Vec4 ZZZW(this Vec4 value) => new Vec4(value.z, value.z, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (z,z,w,0)</summary>
		public static Vec4 ZZW_(this Vec4 value) => new Vec4(value.z, value.z, value.w, 0);

		/// <summary>A transpose swizzle property, returns (z,z,w,x)</summary>
		public static Vec4 ZZWX(this Vec4 value) => new Vec4(value.z, value.z, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (z,z,w,y)</summary>
		public static Vec4 ZZWY(this Vec4 value) => new Vec4(value.z, value.z, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (z,z,w,z)</summary>
		public static Vec4 ZZWZ(this Vec4 value) => new Vec4(value.z, value.z, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (z,z,w,w)</summary>
		public static Vec4 ZZWW(this Vec4 value) => new Vec4(value.z, value.z, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (z,w,0,0)</summary>
		public static Vec4 ZW__(this Vec4 value) => new Vec4(value.z, value.w, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,w,0,x)</summary>
		public static Vec4 ZW_X(this Vec4 value) => new Vec4(value.z, value.w, 0, value.x);

		/// <summary>A transpose swizzle property, returns (z,w,0,y)</summary>
		public static Vec4 ZW_Y(this Vec4 value) => new Vec4(value.z, value.w, 0, value.y);

		/// <summary>A transpose swizzle property, returns (z,w,0,z)</summary>
		public static Vec4 ZW_Z(this Vec4 value) => new Vec4(value.z, value.w, 0, value.z);

		/// <summary>A transpose swizzle property, returns (z,w,0,w)</summary>
		public static Vec4 ZW_W(this Vec4 value) => new Vec4(value.z, value.w, 0, value.w);

		/// <summary>A transpose swizzle property, returns (z,w,x,0)</summary>
		public static Vec4 ZWX_(this Vec4 value) => new Vec4(value.z, value.w, value.x, 0);

		/// <summary>A transpose swizzle property, returns (z,w,x,x)</summary>
		public static Vec4 ZWXX(this Vec4 value) => new Vec4(value.z, value.w, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (z,w,x,y)</summary>
		public static Vec4 ZWXY(this Vec4 value) => new Vec4(value.z, value.w, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (z,w,x,z)</summary>
		public static Vec4 ZWXZ(this Vec4 value) => new Vec4(value.z, value.w, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (z,w,x,w)</summary>
		public static Vec4 ZWXW(this Vec4 value) => new Vec4(value.z, value.w, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (z,w,y,0)</summary>
		public static Vec4 ZWY_(this Vec4 value) => new Vec4(value.z, value.w, value.y, 0);

		/// <summary>A transpose swizzle property, returns (z,w,y,x)</summary>
		public static Vec4 ZWYX(this Vec4 value) => new Vec4(value.z, value.w, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (z,w,y,y)</summary>
		public static Vec4 ZWYY(this Vec4 value) => new Vec4(value.z, value.w, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (z,w,y,z)</summary>
		public static Vec4 ZWYZ(this Vec4 value) => new Vec4(value.z, value.w, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (z,w,y,w)</summary>
		public static Vec4 ZWYW(this Vec4 value) => new Vec4(value.z, value.w, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (z,w,z,0)</summary>
		public static Vec4 ZWZ_(this Vec4 value) => new Vec4(value.z, value.w, value.z, 0);

		/// <summary>A transpose swizzle property, returns (z,w,z,x)</summary>
		public static Vec4 ZWZX(this Vec4 value) => new Vec4(value.z, value.w, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (z,w,z,y)</summary>
		public static Vec4 ZWZY(this Vec4 value) => new Vec4(value.z, value.w, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (z,w,z,z)</summary>
		public static Vec4 ZWZZ(this Vec4 value) => new Vec4(value.z, value.w, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (z,w,z,w)</summary>
		public static Vec4 ZWZW(this Vec4 value) => new Vec4(value.z, value.w, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (z,w,w,0)</summary>
		public static Vec4 ZWW_(this Vec4 value) => new Vec4(value.z, value.w, value.w, 0);

		/// <summary>A transpose swizzle property, returns (z,w,w,x)</summary>
		public static Vec4 ZWWX(this Vec4 value) => new Vec4(value.z, value.w, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (z,w,w,y)</summary>
		public static Vec4 ZWWY(this Vec4 value) => new Vec4(value.z, value.w, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (z,w,w,z)</summary>
		public static Vec4 ZWWZ(this Vec4 value) => new Vec4(value.z, value.w, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (z,w,w,w)</summary>
		public static Vec4 ZWWW(this Vec4 value) => new Vec4(value.z, value.w, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (w,0,0,0)</summary>
		public static Vec4 W___(this Vec4 value) => new Vec4(value.w, 0, 0, 0);

		/// <summary>A transpose swizzle property, returns (w,0,0,x)</summary>
		public static Vec4 W__X(this Vec4 value) => new Vec4(value.w, 0, 0, value.x);

		/// <summary>A transpose swizzle property, returns (w,0,0,y)</summary>
		public static Vec4 W__Y(this Vec4 value) => new Vec4(value.w, 0, 0, value.y);

		/// <summary>A transpose swizzle property, returns (w,0,0,z)</summary>
		public static Vec4 W__Z(this Vec4 value) => new Vec4(value.w, 0, 0, value.z);

		/// <summary>A transpose swizzle property, returns (w,0,0,w)</summary>
		public static Vec4 W__W(this Vec4 value) => new Vec4(value.w, 0, 0, value.w);

		/// <summary>A transpose swizzle property, returns (w,0,x,0)</summary>
		public static Vec4 W_X_(this Vec4 value) => new Vec4(value.w, 0, value.x, 0);

		/// <summary>A transpose swizzle property, returns (w,0,x,x)</summary>
		public static Vec4 W_XX(this Vec4 value) => new Vec4(value.w, 0, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (w,0,x,y)</summary>
		public static Vec4 W_XY(this Vec4 value) => new Vec4(value.w, 0, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (w,0,x,z)</summary>
		public static Vec4 W_XZ(this Vec4 value) => new Vec4(value.w, 0, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (w,0,x,w)</summary>
		public static Vec4 W_XW(this Vec4 value) => new Vec4(value.w, 0, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (w,0,y,0)</summary>
		public static Vec4 W_Y_(this Vec4 value) => new Vec4(value.w, 0, value.y, 0);

		/// <summary>A transpose swizzle property, returns (w,0,y,x)</summary>
		public static Vec4 W_YX(this Vec4 value) => new Vec4(value.w, 0, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (w,0,y,y)</summary>
		public static Vec4 W_YY(this Vec4 value) => new Vec4(value.w, 0, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (w,0,y,z)</summary>
		public static Vec4 W_YZ(this Vec4 value) => new Vec4(value.w, 0, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (w,0,y,w)</summary>
		public static Vec4 W_YW(this Vec4 value) => new Vec4(value.w, 0, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (w,0,z,0)</summary>
		public static Vec4 W_Z_(this Vec4 value) => new Vec4(value.w, 0, value.z, 0);

		/// <summary>A transpose swizzle property, returns (w,0,z,x)</summary>
		public static Vec4 W_ZX(this Vec4 value) => new Vec4(value.w, 0, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (w,0,z,y)</summary>
		public static Vec4 W_ZY(this Vec4 value) => new Vec4(value.w, 0, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (w,0,z,z)</summary>
		public static Vec4 W_ZZ(this Vec4 value) => new Vec4(value.w, 0, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (w,0,z,w)</summary>
		public static Vec4 W_ZW(this Vec4 value) => new Vec4(value.w, 0, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (w,0,w,0)</summary>
		public static Vec4 W_W_(this Vec4 value) => new Vec4(value.w, 0, value.w, 0);

		/// <summary>A transpose swizzle property, returns (w,0,w,x)</summary>
		public static Vec4 W_WX(this Vec4 value) => new Vec4(value.w, 0, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (w,0,w,y)</summary>
		public static Vec4 W_WY(this Vec4 value) => new Vec4(value.w, 0, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (w,0,w,z)</summary>
		public static Vec4 W_WZ(this Vec4 value) => new Vec4(value.w, 0, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (w,0,w,w)</summary>
		public static Vec4 W_WW(this Vec4 value) => new Vec4(value.w, 0, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (w,x,0,0)</summary>
		public static Vec4 WX__(this Vec4 value) => new Vec4(value.w, value.x, 0, 0);

		/// <summary>A transpose swizzle property, returns (w,x,0,x)</summary>
		public static Vec4 WX_X(this Vec4 value) => new Vec4(value.w, value.x, 0, value.x);

		/// <summary>A transpose swizzle property, returns (w,x,0,y)</summary>
		public static Vec4 WX_Y(this Vec4 value) => new Vec4(value.w, value.x, 0, value.y);

		/// <summary>A transpose swizzle property, returns (w,x,0,z)</summary>
		public static Vec4 WX_Z(this Vec4 value) => new Vec4(value.w, value.x, 0, value.z);

		/// <summary>A transpose swizzle property, returns (w,x,0,w)</summary>
		public static Vec4 WX_W(this Vec4 value) => new Vec4(value.w, value.x, 0, value.w);

		/// <summary>A transpose swizzle property, returns (w,x,x,0)</summary>
		public static Vec4 WXX_(this Vec4 value) => new Vec4(value.w, value.x, value.x, 0);

		/// <summary>A transpose swizzle property, returns (w,x,x,x)</summary>
		public static Vec4 WXXX(this Vec4 value) => new Vec4(value.w, value.x, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (w,x,x,y)</summary>
		public static Vec4 WXXY(this Vec4 value) => new Vec4(value.w, value.x, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (w,x,x,z)</summary>
		public static Vec4 WXXZ(this Vec4 value) => new Vec4(value.w, value.x, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (w,x,x,w)</summary>
		public static Vec4 WXXW(this Vec4 value) => new Vec4(value.w, value.x, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (w,x,y,0)</summary>
		public static Vec4 WXY_(this Vec4 value) => new Vec4(value.w, value.x, value.y, 0);

		/// <summary>A transpose swizzle property, returns (w,x,y,x)</summary>
		public static Vec4 WXYX(this Vec4 value) => new Vec4(value.w, value.x, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (w,x,y,y)</summary>
		public static Vec4 WXYY(this Vec4 value) => new Vec4(value.w, value.x, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (w,x,y,z)</summary>
		public static Vec4 WXYZ(this Vec4 value) => new Vec4(value.w, value.x, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (w,x,y,w)</summary>
		public static Vec4 WXYW(this Vec4 value) => new Vec4(value.w, value.x, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (w,x,z,0)</summary>
		public static Vec4 WXZ_(this Vec4 value) => new Vec4(value.w, value.x, value.z, 0);

		/// <summary>A transpose swizzle property, returns (w,x,z,x)</summary>
		public static Vec4 WXZX(this Vec4 value) => new Vec4(value.w, value.x, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (w,x,z,y)</summary>
		public static Vec4 WXZY(this Vec4 value) => new Vec4(value.w, value.x, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (w,x,z,z)</summary>
		public static Vec4 WXZZ(this Vec4 value) => new Vec4(value.w, value.x, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (w,x,z,w)</summary>
		public static Vec4 WXZW(this Vec4 value) => new Vec4(value.w, value.x, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (w,x,w,0)</summary>
		public static Vec4 WXW_(this Vec4 value) => new Vec4(value.w, value.x, value.w, 0);

		/// <summary>A transpose swizzle property, returns (w,x,w,x)</summary>
		public static Vec4 WXWX(this Vec4 value) => new Vec4(value.w, value.x, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (w,x,w,y)</summary>
		public static Vec4 WXWY(this Vec4 value) => new Vec4(value.w, value.x, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (w,x,w,z)</summary>
		public static Vec4 WXWZ(this Vec4 value) => new Vec4(value.w, value.x, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (w,x,w,w)</summary>
		public static Vec4 WXWW(this Vec4 value) => new Vec4(value.w, value.x, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (w,y,0,0)</summary>
		public static Vec4 WY__(this Vec4 value) => new Vec4(value.w, value.y, 0, 0);

		/// <summary>A transpose swizzle property, returns (w,y,0,x)</summary>
		public static Vec4 WY_X(this Vec4 value) => new Vec4(value.w, value.y, 0, value.x);

		/// <summary>A transpose swizzle property, returns (w,y,0,y)</summary>
		public static Vec4 WY_Y(this Vec4 value) => new Vec4(value.w, value.y, 0, value.y);

		/// <summary>A transpose swizzle property, returns (w,y,0,z)</summary>
		public static Vec4 WY_Z(this Vec4 value) => new Vec4(value.w, value.y, 0, value.z);

		/// <summary>A transpose swizzle property, returns (w,y,0,w)</summary>
		public static Vec4 WY_W(this Vec4 value) => new Vec4(value.w, value.y, 0, value.w);

		/// <summary>A transpose swizzle property, returns (w,y,x,0)</summary>
		public static Vec4 WYX_(this Vec4 value) => new Vec4(value.w, value.y, value.x, 0);

		/// <summary>A transpose swizzle property, returns (w,y,x,x)</summary>
		public static Vec4 WYXX(this Vec4 value) => new Vec4(value.w, value.y, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (w,y,x,y)</summary>
		public static Vec4 WYXY(this Vec4 value) => new Vec4(value.w, value.y, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (w,y,x,z)</summary>
		public static Vec4 WYXZ(this Vec4 value) => new Vec4(value.w, value.y, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (w,y,x,w)</summary>
		public static Vec4 WYXW(this Vec4 value) => new Vec4(value.w, value.y, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (w,y,y,0)</summary>
		public static Vec4 WYY_(this Vec4 value) => new Vec4(value.w, value.y, value.y, 0);

		/// <summary>A transpose swizzle property, returns (w,y,y,x)</summary>
		public static Vec4 WYYX(this Vec4 value) => new Vec4(value.w, value.y, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (w,y,y,y)</summary>
		public static Vec4 WYYY(this Vec4 value) => new Vec4(value.w, value.y, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (w,y,y,z)</summary>
		public static Vec4 WYYZ(this Vec4 value) => new Vec4(value.w, value.y, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (w,y,y,w)</summary>
		public static Vec4 WYYW(this Vec4 value) => new Vec4(value.w, value.y, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (w,y,z,0)</summary>
		public static Vec4 WYZ_(this Vec4 value) => new Vec4(value.w, value.y, value.z, 0);

		/// <summary>A transpose swizzle property, returns (w,y,z,x)</summary>
		public static Vec4 WYZX(this Vec4 value) => new Vec4(value.w, value.y, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (w,y,z,y)</summary>
		public static Vec4 WYZY(this Vec4 value) => new Vec4(value.w, value.y, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (w,y,z,z)</summary>
		public static Vec4 WYZZ(this Vec4 value) => new Vec4(value.w, value.y, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (w,y,z,w)</summary>
		public static Vec4 WYZW(this Vec4 value) => new Vec4(value.w, value.y, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (w,y,w,0)</summary>
		public static Vec4 WYW_(this Vec4 value) => new Vec4(value.w, value.y, value.w, 0);

		/// <summary>A transpose swizzle property, returns (w,y,w,x)</summary>
		public static Vec4 WYWX(this Vec4 value) => new Vec4(value.w, value.y, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (w,y,w,y)</summary>
		public static Vec4 WYWY(this Vec4 value) => new Vec4(value.w, value.y, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (w,y,w,z)</summary>
		public static Vec4 WYWZ(this Vec4 value) => new Vec4(value.w, value.y, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (w,y,w,w)</summary>
		public static Vec4 WYWW(this Vec4 value) => new Vec4(value.w, value.y, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (w,z,0,0)</summary>
		public static Vec4 WZ__(this Vec4 value) => new Vec4(value.w, value.z, 0, 0);

		/// <summary>A transpose swizzle property, returns (w,z,0,x)</summary>
		public static Vec4 WZ_X(this Vec4 value) => new Vec4(value.w, value.z, 0, value.x);

		/// <summary>A transpose swizzle property, returns (w,z,0,y)</summary>
		public static Vec4 WZ_Y(this Vec4 value) => new Vec4(value.w, value.z, 0, value.y);

		/// <summary>A transpose swizzle property, returns (w,z,0,z)</summary>
		public static Vec4 WZ_Z(this Vec4 value) => new Vec4(value.w, value.z, 0, value.z);

		/// <summary>A transpose swizzle property, returns (w,z,0,w)</summary>
		public static Vec4 WZ_W(this Vec4 value) => new Vec4(value.w, value.z, 0, value.w);

		/// <summary>A transpose swizzle property, returns (w,z,x,0)</summary>
		public static Vec4 WZX_(this Vec4 value) => new Vec4(value.w, value.z, value.x, 0);

		/// <summary>A transpose swizzle property, returns (w,z,x,x)</summary>
		public static Vec4 WZXX(this Vec4 value) => new Vec4(value.w, value.z, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (w,z,x,y)</summary>
		public static Vec4 WZXY(this Vec4 value) => new Vec4(value.w, value.z, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (w,z,x,z)</summary>
		public static Vec4 WZXZ(this Vec4 value) => new Vec4(value.w, value.z, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (w,z,x,w)</summary>
		public static Vec4 WZXW(this Vec4 value) => new Vec4(value.w, value.z, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (w,z,y,0)</summary>
		public static Vec4 WZY_(this Vec4 value) => new Vec4(value.w, value.z, value.y, 0);

		/// <summary>A transpose swizzle property, returns (w,z,y,x)</summary>
		public static Vec4 WZYX(this Vec4 value) => new Vec4(value.w, value.z, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (w,z,y,y)</summary>
		public static Vec4 WZYY(this Vec4 value) => new Vec4(value.w, value.z, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (w,z,y,z)</summary>
		public static Vec4 WZYZ(this Vec4 value) => new Vec4(value.w, value.z, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (w,z,y,w)</summary>
		public static Vec4 WZYW(this Vec4 value) => new Vec4(value.w, value.z, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (w,z,z,0)</summary>
		public static Vec4 WZZ_(this Vec4 value) => new Vec4(value.w, value.z, value.z, 0);

		/// <summary>A transpose swizzle property, returns (w,z,z,x)</summary>
		public static Vec4 WZZX(this Vec4 value) => new Vec4(value.w, value.z, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (w,z,z,y)</summary>
		public static Vec4 WZZY(this Vec4 value) => new Vec4(value.w, value.z, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (w,z,z,z)</summary>
		public static Vec4 WZZZ(this Vec4 value) => new Vec4(value.w, value.z, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (w,z,z,w)</summary>
		public static Vec4 WZZW(this Vec4 value) => new Vec4(value.w, value.z, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (w,z,w,0)</summary>
		public static Vec4 WZW_(this Vec4 value) => new Vec4(value.w, value.z, value.w, 0);

		/// <summary>A transpose swizzle property, returns (w,z,w,x)</summary>
		public static Vec4 WZWX(this Vec4 value) => new Vec4(value.w, value.z, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (w,z,w,y)</summary>
		public static Vec4 WZWY(this Vec4 value) => new Vec4(value.w, value.z, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (w,z,w,z)</summary>
		public static Vec4 WZWZ(this Vec4 value) => new Vec4(value.w, value.z, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (w,z,w,w)</summary>
		public static Vec4 WZWW(this Vec4 value) => new Vec4(value.w, value.z, value.w, value.w);

		/// <summary>A transpose swizzle property, returns (w,w,0,0)</summary>
		public static Vec4 WW__(this Vec4 value) => new Vec4(value.w, value.w, 0, 0);

		/// <summary>A transpose swizzle property, returns (w,w,0,x)</summary>
		public static Vec4 WW_X(this Vec4 value) => new Vec4(value.w, value.w, 0, value.x);

		/// <summary>A transpose swizzle property, returns (w,w,0,y)</summary>
		public static Vec4 WW_Y(this Vec4 value) => new Vec4(value.w, value.w, 0, value.y);

		/// <summary>A transpose swizzle property, returns (w,w,0,z)</summary>
		public static Vec4 WW_Z(this Vec4 value) => new Vec4(value.w, value.w, 0, value.z);

		/// <summary>A transpose swizzle property, returns (w,w,0,w)</summary>
		public static Vec4 WW_W(this Vec4 value) => new Vec4(value.w, value.w, 0, value.w);

		/// <summary>A transpose swizzle property, returns (w,w,x,0)</summary>
		public static Vec4 WWX_(this Vec4 value) => new Vec4(value.w, value.w, value.x, 0);

		/// <summary>A transpose swizzle property, returns (w,w,x,x)</summary>
		public static Vec4 WWXX(this Vec4 value) => new Vec4(value.w, value.w, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (w,w,x,y)</summary>
		public static Vec4 WWXY(this Vec4 value) => new Vec4(value.w, value.w, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (w,w,x,z)</summary>
		public static Vec4 WWXZ(this Vec4 value) => new Vec4(value.w, value.w, value.x, value.z);

		/// <summary>A transpose swizzle property, returns (w,w,x,w)</summary>
		public static Vec4 WWXW(this Vec4 value) => new Vec4(value.w, value.w, value.x, value.w);

		/// <summary>A transpose swizzle property, returns (w,w,y,0)</summary>
		public static Vec4 WWY_(this Vec4 value) => new Vec4(value.w, value.w, value.y, 0);

		/// <summary>A transpose swizzle property, returns (w,w,y,x)</summary>
		public static Vec4 WWYX(this Vec4 value) => new Vec4(value.w, value.w, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (w,w,y,y)</summary>
		public static Vec4 WWYY(this Vec4 value) => new Vec4(value.w, value.w, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (w,w,y,z)</summary>
		public static Vec4 WWYZ(this Vec4 value) => new Vec4(value.w, value.w, value.y, value.z);

		/// <summary>A transpose swizzle property, returns (w,w,y,w)</summary>
		public static Vec4 WWYW(this Vec4 value) => new Vec4(value.w, value.w, value.y, value.w);

		/// <summary>A transpose swizzle property, returns (w,w,z,0)</summary>
		public static Vec4 WWZ_(this Vec4 value) => new Vec4(value.w, value.w, value.z, 0);

		/// <summary>A transpose swizzle property, returns (w,w,z,x)</summary>
		public static Vec4 WWZX(this Vec4 value) => new Vec4(value.w, value.w, value.z, value.x);

		/// <summary>A transpose swizzle property, returns (w,w,z,y)</summary>
		public static Vec4 WWZY(this Vec4 value) => new Vec4(value.w, value.w, value.z, value.y);

		/// <summary>A transpose swizzle property, returns (w,w,z,z)</summary>
		public static Vec4 WWZZ(this Vec4 value) => new Vec4(value.w, value.w, value.z, value.z);

		/// <summary>A transpose swizzle property, returns (w,w,z,w)</summary>
		public static Vec4 WWZW(this Vec4 value) => new Vec4(value.w, value.w, value.z, value.w);

		/// <summary>A transpose swizzle property, returns (w,w,w,0)</summary>
		public static Vec4 WWW_(this Vec4 value) => new Vec4(value.w, value.w, value.w, 0);

		/// <summary>A transpose swizzle property, returns (w,w,w,x)</summary>
		public static Vec4 WWWX(this Vec4 value) => new Vec4(value.w, value.w, value.w, value.x);

		/// <summary>A transpose swizzle property, returns (w,w,w,y)</summary>
		public static Vec4 WWWY(this Vec4 value) => new Vec4(value.w, value.w, value.w, value.y);

		/// <summary>A transpose swizzle property, returns (w,w,w,z)</summary>
		public static Vec4 WWWZ(this Vec4 value) => new Vec4(value.w, value.w, value.w, value.z);

		/// <summary>A transpose swizzle property, returns (w,w,w,w)</summary>
		public static Vec4 WWWW(this Vec4 value) => new Vec4(value.w, value.w, value.w, value.w);

	}

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



	/// <summary>Swizzels for Vec2</summary>
	public static class Vec2Swizzel
	{

		/// <summary>A transpose swizzle property, returns (0,x)</summary>
		public static Vec2 _X(this Vec2 value) => new Vec2(0, value.x);

		/// <summary>A transpose swizzle property, returns (0,y)</summary>
		public static Vec2 _Y(this Vec2 value) => new Vec2(0, value.y);

		/// <summary>A transpose swizzle property, returns (x,0)</summary>
		public static Vec2 X_(this Vec2 value) => new Vec2(value.x, 0);

		/// <summary>A transpose swizzle property, returns (x,x)</summary>
		public static Vec2 XX(this Vec2 value) => new Vec2(value.x, value.x);

		/// <summary>A transpose swizzle property, returns (x,y)</summary>
		public static Vec2 XY(this Vec2 value) => new Vec2(value.x, value.y);

		/// <summary>A transpose swizzle property, returns (y,0)</summary>
		public static Vec2 Y_(this Vec2 value) => new Vec2(value.y, 0);

		/// <summary>A transpose swizzle property, returns (y,x)</summary>
		public static Vec2 YX(this Vec2 value) => new Vec2(value.y, value.x);

		/// <summary>A transpose swizzle property, returns (y,y)</summary>
		public static Vec2 YY(this Vec2 value) => new Vec2(value.y, value.y);

		/// <summary>A transpose swizzle property, returns (0,0,x)</summary>
		public static Vec3 __X(this Vec2 value) => new Vec3(0, 0, value.x);

		/// <summary>A transpose swizzle property, returns (0,0,y)</summary>
		public static Vec3 __Y(this Vec2 value) => new Vec3(0, 0, value.y);

		/// <summary>A transpose swizzle property, returns (0,x,0)</summary>
		public static Vec3 _X_(this Vec2 value) => new Vec3(0, value.x, 0);

		/// <summary>A transpose swizzle property, returns (0,x,x)</summary>
		public static Vec3 _XX(this Vec2 value) => new Vec3(0, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (0,x,y)</summary>
		public static Vec3 _XY(this Vec2 value) => new Vec3(0, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (0,y,0)</summary>
		public static Vec3 _Y_(this Vec2 value) => new Vec3(0, value.y, 0);

		/// <summary>A transpose swizzle property, returns (0,y,x)</summary>
		public static Vec3 _YX(this Vec2 value) => new Vec3(0, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (0,y,y)</summary>
		public static Vec3 _YY(this Vec2 value) => new Vec3(0, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (x,0,0)</summary>
		public static Vec3 X__(this Vec2 value) => new Vec3(value.x, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,0,x)</summary>
		public static Vec3 X_X(this Vec2 value) => new Vec3(value.x, 0, value.x);

		/// <summary>A transpose swizzle property, returns (x,0,y)</summary>
		public static Vec3 X_Y(this Vec2 value) => new Vec3(value.x, 0, value.y);

		/// <summary>A transpose swizzle property, returns (x,x,0)</summary>
		public static Vec3 XX_(this Vec2 value) => new Vec3(value.x, value.x, 0);

		/// <summary>A transpose swizzle property, returns (x,x,x)</summary>
		public static Vec3 XXX(this Vec2 value) => new Vec3(value.x, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (x,x,y)</summary>
		public static Vec3 XXY(this Vec2 value) => new Vec3(value.x, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (x,y,0)</summary>
		public static Vec3 XY_(this Vec2 value) => new Vec3(value.x, value.y, 0);

		/// <summary>A transpose swizzle property, returns (x,y,x)</summary>
		public static Vec3 XYX(this Vec2 value) => new Vec3(value.x, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (x,y,y)</summary>
		public static Vec3 XYY(this Vec2 value) => new Vec3(value.x, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (y,0,0)</summary>
		public static Vec3 Y__(this Vec2 value) => new Vec3(value.y, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,0,x)</summary>
		public static Vec3 Y_X(this Vec2 value) => new Vec3(value.y, 0, value.x);

		/// <summary>A transpose swizzle property, returns (y,0,y)</summary>
		public static Vec3 Y_Y(this Vec2 value) => new Vec3(value.y, 0, value.y);

		/// <summary>A transpose swizzle property, returns (y,x,0)</summary>
		public static Vec3 YX_(this Vec2 value) => new Vec3(value.y, value.x, 0);

		/// <summary>A transpose swizzle property, returns (y,x,x)</summary>
		public static Vec3 YXX(this Vec2 value) => new Vec3(value.y, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (y,x,y)</summary>
		public static Vec3 YXY(this Vec2 value) => new Vec3(value.y, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (y,y,0)</summary>
		public static Vec3 YY_(this Vec2 value) => new Vec3(value.y, value.y, 0);

		/// <summary>A transpose swizzle property, returns (y,y,x)</summary>
		public static Vec3 YYX(this Vec2 value) => new Vec3(value.y, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (y,y,y)</summary>
		public static Vec3 YYY(this Vec2 value) => new Vec3(value.y, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (0,0,0,x)</summary>
		public static Vec4 ___X(this Vec2 value) => new Vec4(0, 0, 0, value.x);

		/// <summary>A transpose swizzle property, returns (0,0,0,y)</summary>
		public static Vec4 ___Y(this Vec2 value) => new Vec4(0, 0, 0, value.y);

		/// <summary>A transpose swizzle property, returns (0,0,x,0)</summary>
		public static Vec4 __X_(this Vec2 value) => new Vec4(0, 0, value.x, 0);

		/// <summary>A transpose swizzle property, returns (0,0,x,x)</summary>
		public static Vec4 __XX(this Vec2 value) => new Vec4(0, 0, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (0,0,x,y)</summary>
		public static Vec4 __XY(this Vec2 value) => new Vec4(0, 0, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (0,0,y,0)</summary>
		public static Vec4 __Y_(this Vec2 value) => new Vec4(0, 0, value.y, 0);

		/// <summary>A transpose swizzle property, returns (0,0,y,x)</summary>
		public static Vec4 __YX(this Vec2 value) => new Vec4(0, 0, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (0,0,y,y)</summary>
		public static Vec4 __YY(this Vec2 value) => new Vec4(0, 0, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (0,x,0,0)</summary>
		public static Vec4 _X__(this Vec2 value) => new Vec4(0, value.x, 0, 0);

		/// <summary>A transpose swizzle property, returns (0,x,0,x)</summary>
		public static Vec4 _X_X(this Vec2 value) => new Vec4(0, value.x, 0, value.x);

		/// <summary>A transpose swizzle property, returns (0,x,0,y)</summary>
		public static Vec4 _X_Y(this Vec2 value) => new Vec4(0, value.x, 0, value.y);

		/// <summary>A transpose swizzle property, returns (0,x,x,0)</summary>
		public static Vec4 _XX_(this Vec2 value) => new Vec4(0, value.x, value.x, 0);

		/// <summary>A transpose swizzle property, returns (0,x,x,x)</summary>
		public static Vec4 _XXX(this Vec2 value) => new Vec4(0, value.x, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (0,x,x,y)</summary>
		public static Vec4 _XXY(this Vec2 value) => new Vec4(0, value.x, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (0,x,y,0)</summary>
		public static Vec4 _XY_(this Vec2 value) => new Vec4(0, value.x, value.y, 0);

		/// <summary>A transpose swizzle property, returns (0,x,y,x)</summary>
		public static Vec4 _XYX(this Vec2 value) => new Vec4(0, value.x, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (0,x,y,y)</summary>
		public static Vec4 _XYY(this Vec2 value) => new Vec4(0, value.x, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (0,y,0,0)</summary>
		public static Vec4 _Y__(this Vec2 value) => new Vec4(0, value.y, 0, 0);

		/// <summary>A transpose swizzle property, returns (0,y,0,x)</summary>
		public static Vec4 _Y_X(this Vec2 value) => new Vec4(0, value.y, 0, value.x);

		/// <summary>A transpose swizzle property, returns (0,y,0,y)</summary>
		public static Vec4 _Y_Y(this Vec2 value) => new Vec4(0, value.y, 0, value.y);

		/// <summary>A transpose swizzle property, returns (0,y,x,0)</summary>
		public static Vec4 _YX_(this Vec2 value) => new Vec4(0, value.y, value.x, 0);

		/// <summary>A transpose swizzle property, returns (0,y,x,x)</summary>
		public static Vec4 _YXX(this Vec2 value) => new Vec4(0, value.y, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (0,y,x,y)</summary>
		public static Vec4 _YXY(this Vec2 value) => new Vec4(0, value.y, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (0,y,y,0)</summary>
		public static Vec4 _YY_(this Vec2 value) => new Vec4(0, value.y, value.y, 0);

		/// <summary>A transpose swizzle property, returns (0,y,y,x)</summary>
		public static Vec4 _YYX(this Vec2 value) => new Vec4(0, value.y, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (0,y,y,y)</summary>
		public static Vec4 _YYY(this Vec2 value) => new Vec4(0, value.y, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (x,0,0,0)</summary>
		public static Vec4 X___(this Vec2 value) => new Vec4(value.x, 0, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,0,0,x)</summary>
		public static Vec4 X__X(this Vec2 value) => new Vec4(value.x, 0, 0, value.x);

		/// <summary>A transpose swizzle property, returns (x,0,0,y)</summary>
		public static Vec4 X__Y(this Vec2 value) => new Vec4(value.x, 0, 0, value.y);

		/// <summary>A transpose swizzle property, returns (x,0,x,0)</summary>
		public static Vec4 X_X_(this Vec2 value) => new Vec4(value.x, 0, value.x, 0);

		/// <summary>A transpose swizzle property, returns (x,0,x,x)</summary>
		public static Vec4 X_XX(this Vec2 value) => new Vec4(value.x, 0, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (x,0,x,y)</summary>
		public static Vec4 X_XY(this Vec2 value) => new Vec4(value.x, 0, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (x,0,y,0)</summary>
		public static Vec4 X_Y_(this Vec2 value) => new Vec4(value.x, 0, value.y, 0);

		/// <summary>A transpose swizzle property, returns (x,0,y,x)</summary>
		public static Vec4 X_YX(this Vec2 value) => new Vec4(value.x, 0, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (x,0,y,y)</summary>
		public static Vec4 X_YY(this Vec2 value) => new Vec4(value.x, 0, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (x,x,0,0)</summary>
		public static Vec4 XX__(this Vec2 value) => new Vec4(value.x, value.x, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,x,0,x)</summary>
		public static Vec4 XX_X(this Vec2 value) => new Vec4(value.x, value.x, 0, value.x);

		/// <summary>A transpose swizzle property, returns (x,x,0,y)</summary>
		public static Vec4 XX_Y(this Vec2 value) => new Vec4(value.x, value.x, 0, value.y);

		/// <summary>A transpose swizzle property, returns (x,x,x,0)</summary>
		public static Vec4 XXX_(this Vec2 value) => new Vec4(value.x, value.x, value.x, 0);

		/// <summary>A transpose swizzle property, returns (x,x,x,x)</summary>
		public static Vec4 XXXX(this Vec2 value) => new Vec4(value.x, value.x, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (x,x,x,y)</summary>
		public static Vec4 XXXY(this Vec2 value) => new Vec4(value.x, value.x, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (x,x,y,0)</summary>
		public static Vec4 XXY_(this Vec2 value) => new Vec4(value.x, value.x, value.y, 0);

		/// <summary>A transpose swizzle property, returns (x,x,y,x)</summary>
		public static Vec4 XXYX(this Vec2 value) => new Vec4(value.x, value.x, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (x,x,y,y)</summary>
		public static Vec4 XXYY(this Vec2 value) => new Vec4(value.x, value.x, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (x,y,0,0)</summary>
		public static Vec4 XY__(this Vec2 value) => new Vec4(value.x, value.y, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,y,0,x)</summary>
		public static Vec4 XY_X(this Vec2 value) => new Vec4(value.x, value.y, 0, value.x);

		/// <summary>A transpose swizzle property, returns (x,y,0,y)</summary>
		public static Vec4 XY_Y(this Vec2 value) => new Vec4(value.x, value.y, 0, value.y);

		/// <summary>A transpose swizzle property, returns (x,y,x,0)</summary>
		public static Vec4 XYX_(this Vec2 value) => new Vec4(value.x, value.y, value.x, 0);

		/// <summary>A transpose swizzle property, returns (x,y,x,x)</summary>
		public static Vec4 XYXX(this Vec2 value) => new Vec4(value.x, value.y, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (x,y,x,y)</summary>
		public static Vec4 XYXY(this Vec2 value) => new Vec4(value.x, value.y, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (x,y,y,0)</summary>
		public static Vec4 XYY_(this Vec2 value) => new Vec4(value.x, value.y, value.y, 0);

		/// <summary>A transpose swizzle property, returns (x,y,y,x)</summary>
		public static Vec4 XYYX(this Vec2 value) => new Vec4(value.x, value.y, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (x,y,y,y)</summary>
		public static Vec4 XYYY(this Vec2 value) => new Vec4(value.x, value.y, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (y,0,0,0)</summary>
		public static Vec4 Y___(this Vec2 value) => new Vec4(value.y, 0, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,0,0,x)</summary>
		public static Vec4 Y__X(this Vec2 value) => new Vec4(value.y, 0, 0, value.x);

		/// <summary>A transpose swizzle property, returns (y,0,0,y)</summary>
		public static Vec4 Y__Y(this Vec2 value) => new Vec4(value.y, 0, 0, value.y);

		/// <summary>A transpose swizzle property, returns (y,0,x,0)</summary>
		public static Vec4 Y_X_(this Vec2 value) => new Vec4(value.y, 0, value.x, 0);

		/// <summary>A transpose swizzle property, returns (y,0,x,x)</summary>
		public static Vec4 Y_XX(this Vec2 value) => new Vec4(value.y, 0, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (y,0,x,y)</summary>
		public static Vec4 Y_XY(this Vec2 value) => new Vec4(value.y, 0, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (y,0,y,0)</summary>
		public static Vec4 Y_Y_(this Vec2 value) => new Vec4(value.y, 0, value.y, 0);

		/// <summary>A transpose swizzle property, returns (y,0,y,x)</summary>
		public static Vec4 Y_YX(this Vec2 value) => new Vec4(value.y, 0, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (y,0,y,y)</summary>
		public static Vec4 Y_YY(this Vec2 value) => new Vec4(value.y, 0, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (y,x,0,0)</summary>
		public static Vec4 YX__(this Vec2 value) => new Vec4(value.y, value.x, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,x,0,x)</summary>
		public static Vec4 YX_X(this Vec2 value) => new Vec4(value.y, value.x, 0, value.x);

		/// <summary>A transpose swizzle property, returns (y,x,0,y)</summary>
		public static Vec4 YX_Y(this Vec2 value) => new Vec4(value.y, value.x, 0, value.y);

		/// <summary>A transpose swizzle property, returns (y,x,x,0)</summary>
		public static Vec4 YXX_(this Vec2 value) => new Vec4(value.y, value.x, value.x, 0);

		/// <summary>A transpose swizzle property, returns (y,x,x,x)</summary>
		public static Vec4 YXXX(this Vec2 value) => new Vec4(value.y, value.x, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (y,x,x,y)</summary>
		public static Vec4 YXXY(this Vec2 value) => new Vec4(value.y, value.x, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (y,x,y,0)</summary>
		public static Vec4 YXY_(this Vec2 value) => new Vec4(value.y, value.x, value.y, 0);

		/// <summary>A transpose swizzle property, returns (y,x,y,x)</summary>
		public static Vec4 YXYX(this Vec2 value) => new Vec4(value.y, value.x, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (y,x,y,y)</summary>
		public static Vec4 YXYY(this Vec2 value) => new Vec4(value.y, value.x, value.y, value.y);

		/// <summary>A transpose swizzle property, returns (y,y,0,0)</summary>
		public static Vec4 YY__(this Vec2 value) => new Vec4(value.y, value.y, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,y,0,x)</summary>
		public static Vec4 YY_X(this Vec2 value) => new Vec4(value.y, value.y, 0, value.x);

		/// <summary>A transpose swizzle property, returns (y,y,0,y)</summary>
		public static Vec4 YY_Y(this Vec2 value) => new Vec4(value.y, value.y, 0, value.y);

		/// <summary>A transpose swizzle property, returns (y,y,x,0)</summary>
		public static Vec4 YYX_(this Vec2 value) => new Vec4(value.y, value.y, value.x, 0);

		/// <summary>A transpose swizzle property, returns (y,y,x,x)</summary>
		public static Vec4 YYXX(this Vec2 value) => new Vec4(value.y, value.y, value.x, value.x);

		/// <summary>A transpose swizzle property, returns (y,y,x,y)</summary>
		public static Vec4 YYXY(this Vec2 value) => new Vec4(value.y, value.y, value.x, value.y);

		/// <summary>A transpose swizzle property, returns (y,y,y,0)</summary>
		public static Vec4 YYY_(this Vec2 value) => new Vec4(value.y, value.y, value.y, 0);

		/// <summary>A transpose swizzle property, returns (y,y,y,x)</summary>
		public static Vec4 YYYX(this Vec2 value) => new Vec4(value.y, value.y, value.y, value.x);

		/// <summary>A transpose swizzle property, returns (y,y,y,y)</summary>
		public static Vec4 YYYY(this Vec2 value) => new Vec4(value.y, value.y, value.y, value.y);

	}


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
