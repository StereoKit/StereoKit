
using System;

namespace StereoKit.Swizzels
{
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
}
