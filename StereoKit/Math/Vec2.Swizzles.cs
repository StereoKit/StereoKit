
using System;

namespace StereoKit
{
	public partial struct Vec2
	{

		/// <summary>A transpose swizzle property, returns (0,x)</summary>
		public Vec2 OX => new Vec2(0, x);

		/// <summary>A transpose swizzle property, returns (0,y)</summary>
		public Vec2 OY => new Vec2(0, y);

		/// <summary>A transpose swizzle property, returns (x,0)</summary>
		public Vec2 X0 => new Vec2(x, 0);

		/// <summary>A transpose swizzle property, returns (x,x)</summary>
		public Vec2 XX => new Vec2(x, x);

		/// <summary>A transpose swizzle property, returns (x,y)</summary>
		public Vec2 XY => new Vec2(x, y);

		/// <summary>A transpose swizzle property, returns (y,0)</summary>
		public Vec2 Y0 => new Vec2(y, 0);

		/// <summary>A transpose swizzle property, returns (y,x)</summary>
		public Vec2 YX => new Vec2(y, x);

		/// <summary>A transpose swizzle property, returns (y,y)</summary>
		public Vec2 YY => new Vec2(y, y);

		/// <summary>A transpose swizzle property, returns (0,0,x)</summary>
		public Vec3 O0X => new Vec3(0, 0, x);

		/// <summary>A transpose swizzle property, returns (0,0,y)</summary>
		public Vec3 O0Y => new Vec3(0, 0, y);

		/// <summary>A transpose swizzle property, returns (0,x,0)</summary>
		public Vec3 OX0 => new Vec3(0, x, 0);

		/// <summary>A transpose swizzle property, returns (0,x,x)</summary>
		public Vec3 OXX => new Vec3(0, x, x);

		/// <summary>A transpose swizzle property, returns (0,x,y)</summary>
		public Vec3 OXY => new Vec3(0, x, y);

		/// <summary>A transpose swizzle property, returns (0,y,0)</summary>
		public Vec3 OY0 => new Vec3(0, y, 0);

		/// <summary>A transpose swizzle property, returns (0,y,x)</summary>
		public Vec3 OYX => new Vec3(0, y, x);

		/// <summary>A transpose swizzle property, returns (0,y,y)</summary>
		public Vec3 OYY => new Vec3(0, y, y);

		/// <summary>A transpose swizzle property, returns (x,0,0)</summary>
		public Vec3 X00 => new Vec3(x, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,0,x)</summary>
		public Vec3 X0X => new Vec3(x, 0, x);

		/// <summary>A transpose swizzle property, returns (x,0,y)</summary>
		public Vec3 X0Y => new Vec3(x, 0, y);

		/// <summary>A transpose swizzle property, returns (x,x,0)</summary>
		public Vec3 XX0 => new Vec3(x, x, 0);

		/// <summary>A transpose swizzle property, returns (x,x,x)</summary>
		public Vec3 XXX => new Vec3(x, x, x);

		/// <summary>A transpose swizzle property, returns (x,x,y)</summary>
		public Vec3 XXY => new Vec3(x, x, y);

		/// <summary>A transpose swizzle property, returns (x,y,0)</summary>
		public Vec3 XY0 => new Vec3(x, y, 0);

		/// <summary>A transpose swizzle property, returns (x,y,x)</summary>
		public Vec3 XYX => new Vec3(x, y, x);

		/// <summary>A transpose swizzle property, returns (x,y,y)</summary>
		public Vec3 XYY => new Vec3(x, y, y);

		/// <summary>A transpose swizzle property, returns (y,0,0)</summary>
		public Vec3 Y00 => new Vec3(y, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,0,x)</summary>
		public Vec3 Y0X => new Vec3(y, 0, x);

		/// <summary>A transpose swizzle property, returns (y,0,y)</summary>
		public Vec3 Y0Y => new Vec3(y, 0, y);

		/// <summary>A transpose swizzle property, returns (y,x,0)</summary>
		public Vec3 YX0 => new Vec3(y, x, 0);

		/// <summary>A transpose swizzle property, returns (y,x,x)</summary>
		public Vec3 YXX => new Vec3(y, x, x);

		/// <summary>A transpose swizzle property, returns (y,x,y)</summary>
		public Vec3 YXY => new Vec3(y, x, y);

		/// <summary>A transpose swizzle property, returns (y,y,0)</summary>
		public Vec3 YY0 => new Vec3(y, y, 0);

		/// <summary>A transpose swizzle property, returns (y,y,x)</summary>
		public Vec3 YYX => new Vec3(y, y, x);

		/// <summary>A transpose swizzle property, returns (y,y,y)</summary>
		public Vec3 YYY => new Vec3(y, y, y);

		/// <summary>A transpose swizzle property, returns (0,0,0,x)</summary>
		public Vec4 O00X => new Vec4(0, 0, 0, x);

		/// <summary>A transpose swizzle property, returns (0,0,0,y)</summary>
		public Vec4 O00Y => new Vec4(0, 0, 0, y);

		/// <summary>A transpose swizzle property, returns (0,0,x,0)</summary>
		public Vec4 O0X0 => new Vec4(0, 0, x, 0);

		/// <summary>A transpose swizzle property, returns (0,0,x,x)</summary>
		public Vec4 O0XX => new Vec4(0, 0, x, x);

		/// <summary>A transpose swizzle property, returns (0,0,x,y)</summary>
		public Vec4 O0XY => new Vec4(0, 0, x, y);

		/// <summary>A transpose swizzle property, returns (0,0,y,0)</summary>
		public Vec4 O0Y0 => new Vec4(0, 0, y, 0);

		/// <summary>A transpose swizzle property, returns (0,0,y,x)</summary>
		public Vec4 O0YX => new Vec4(0, 0, y, x);

		/// <summary>A transpose swizzle property, returns (0,0,y,y)</summary>
		public Vec4 O0YY => new Vec4(0, 0, y, y);

		/// <summary>A transpose swizzle property, returns (0,x,0,0)</summary>
		public Vec4 OX00 => new Vec4(0, x, 0, 0);

		/// <summary>A transpose swizzle property, returns (0,x,0,x)</summary>
		public Vec4 OX0X => new Vec4(0, x, 0, x);

		/// <summary>A transpose swizzle property, returns (0,x,0,y)</summary>
		public Vec4 OX0Y => new Vec4(0, x, 0, y);

		/// <summary>A transpose swizzle property, returns (0,x,x,0)</summary>
		public Vec4 OXX0 => new Vec4(0, x, x, 0);

		/// <summary>A transpose swizzle property, returns (0,x,x,x)</summary>
		public Vec4 OXXX => new Vec4(0, x, x, x);

		/// <summary>A transpose swizzle property, returns (0,x,x,y)</summary>
		public Vec4 OXXY => new Vec4(0, x, x, y);

		/// <summary>A transpose swizzle property, returns (0,x,y,0)</summary>
		public Vec4 OXY0 => new Vec4(0, x, y, 0);

		/// <summary>A transpose swizzle property, returns (0,x,y,x)</summary>
		public Vec4 OXYX => new Vec4(0, x, y, x);

		/// <summary>A transpose swizzle property, returns (0,x,y,y)</summary>
		public Vec4 OXYY => new Vec4(0, x, y, y);

		/// <summary>A transpose swizzle property, returns (0,y,0,0)</summary>
		public Vec4 OY00 => new Vec4(0, y, 0, 0);

		/// <summary>A transpose swizzle property, returns (0,y,0,x)</summary>
		public Vec4 OY0X => new Vec4(0, y, 0, x);

		/// <summary>A transpose swizzle property, returns (0,y,0,y)</summary>
		public Vec4 OY0Y => new Vec4(0, y, 0, y);

		/// <summary>A transpose swizzle property, returns (0,y,x,0)</summary>
		public Vec4 OYX0 => new Vec4(0, y, x, 0);

		/// <summary>A transpose swizzle property, returns (0,y,x,x)</summary>
		public Vec4 OYXX => new Vec4(0, y, x, x);

		/// <summary>A transpose swizzle property, returns (0,y,x,y)</summary>
		public Vec4 OYXY => new Vec4(0, y, x, y);

		/// <summary>A transpose swizzle property, returns (0,y,y,0)</summary>
		public Vec4 OYY0 => new Vec4(0, y, y, 0);

		/// <summary>A transpose swizzle property, returns (0,y,y,x)</summary>
		public Vec4 OYYX => new Vec4(0, y, y, x);

		/// <summary>A transpose swizzle property, returns (0,y,y,y)</summary>
		public Vec4 OYYY => new Vec4(0, y, y, y);

		/// <summary>A transpose swizzle property, returns (x,0,0,0)</summary>
		public Vec4 X000 => new Vec4(x, 0, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,0,0,x)</summary>
		public Vec4 X00X => new Vec4(x, 0, 0, x);

		/// <summary>A transpose swizzle property, returns (x,0,0,y)</summary>
		public Vec4 X00Y => new Vec4(x, 0, 0, y);

		/// <summary>A transpose swizzle property, returns (x,0,x,0)</summary>
		public Vec4 X0X0 => new Vec4(x, 0, x, 0);

		/// <summary>A transpose swizzle property, returns (x,0,x,x)</summary>
		public Vec4 X0XX => new Vec4(x, 0, x, x);

		/// <summary>A transpose swizzle property, returns (x,0,x,y)</summary>
		public Vec4 X0XY => new Vec4(x, 0, x, y);

		/// <summary>A transpose swizzle property, returns (x,0,y,0)</summary>
		public Vec4 X0Y0 => new Vec4(x, 0, y, 0);

		/// <summary>A transpose swizzle property, returns (x,0,y,x)</summary>
		public Vec4 X0YX => new Vec4(x, 0, y, x);

		/// <summary>A transpose swizzle property, returns (x,0,y,y)</summary>
		public Vec4 X0YY => new Vec4(x, 0, y, y);

		/// <summary>A transpose swizzle property, returns (x,x,0,0)</summary>
		public Vec4 XX00 => new Vec4(x, x, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,x,0,x)</summary>
		public Vec4 XX0X => new Vec4(x, x, 0, x);

		/// <summary>A transpose swizzle property, returns (x,x,0,y)</summary>
		public Vec4 XX0Y => new Vec4(x, x, 0, y);

		/// <summary>A transpose swizzle property, returns (x,x,x,0)</summary>
		public Vec4 XXX0 => new Vec4(x, x, x, 0);

		/// <summary>A transpose swizzle property, returns (x,x,x,x)</summary>
		public Vec4 XXXX => new Vec4(x, x, x, x);

		/// <summary>A transpose swizzle property, returns (x,x,x,y)</summary>
		public Vec4 XXXY => new Vec4(x, x, x, y);

		/// <summary>A transpose swizzle property, returns (x,x,y,0)</summary>
		public Vec4 XXY0 => new Vec4(x, x, y, 0);

		/// <summary>A transpose swizzle property, returns (x,x,y,x)</summary>
		public Vec4 XXYX => new Vec4(x, x, y, x);

		/// <summary>A transpose swizzle property, returns (x,x,y,y)</summary>
		public Vec4 XXYY => new Vec4(x, x, y, y);

		/// <summary>A transpose swizzle property, returns (x,y,0,0)</summary>
		public Vec4 XY00 => new Vec4(x, y, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,y,0,x)</summary>
		public Vec4 XY0X => new Vec4(x, y, 0, x);

		/// <summary>A transpose swizzle property, returns (x,y,0,y)</summary>
		public Vec4 XY0Y => new Vec4(x, y, 0, y);

		/// <summary>A transpose swizzle property, returns (x,y,x,0)</summary>
		public Vec4 XYX0 => new Vec4(x, y, x, 0);

		/// <summary>A transpose swizzle property, returns (x,y,x,x)</summary>
		public Vec4 XYXX => new Vec4(x, y, x, x);

		/// <summary>A transpose swizzle property, returns (x,y,x,y)</summary>
		public Vec4 XYXY => new Vec4(x, y, x, y);

		/// <summary>A transpose swizzle property, returns (x,y,y,0)</summary>
		public Vec4 XYY0 => new Vec4(x, y, y, 0);

		/// <summary>A transpose swizzle property, returns (x,y,y,x)</summary>
		public Vec4 XYYX => new Vec4(x, y, y, x);

		/// <summary>A transpose swizzle property, returns (x,y,y,y)</summary>
		public Vec4 XYYY => new Vec4(x, y, y, y);

		/// <summary>A transpose swizzle property, returns (y,0,0,0)</summary>
		public Vec4 Y000 => new Vec4(y, 0, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,0,0,x)</summary>
		public Vec4 Y00X => new Vec4(y, 0, 0, x);

		/// <summary>A transpose swizzle property, returns (y,0,0,y)</summary>
		public Vec4 Y00Y => new Vec4(y, 0, 0, y);

		/// <summary>A transpose swizzle property, returns (y,0,x,0)</summary>
		public Vec4 Y0X0 => new Vec4(y, 0, x, 0);

		/// <summary>A transpose swizzle property, returns (y,0,x,x)</summary>
		public Vec4 Y0XX => new Vec4(y, 0, x, x);

		/// <summary>A transpose swizzle property, returns (y,0,x,y)</summary>
		public Vec4 Y0XY => new Vec4(y, 0, x, y);

		/// <summary>A transpose swizzle property, returns (y,0,y,0)</summary>
		public Vec4 Y0Y0 => new Vec4(y, 0, y, 0);

		/// <summary>A transpose swizzle property, returns (y,0,y,x)</summary>
		public Vec4 Y0YX => new Vec4(y, 0, y, x);

		/// <summary>A transpose swizzle property, returns (y,0,y,y)</summary>
		public Vec4 Y0YY => new Vec4(y, 0, y, y);

		/// <summary>A transpose swizzle property, returns (y,x,0,0)</summary>
		public Vec4 YX00 => new Vec4(y, x, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,x,0,x)</summary>
		public Vec4 YX0X => new Vec4(y, x, 0, x);

		/// <summary>A transpose swizzle property, returns (y,x,0,y)</summary>
		public Vec4 YX0Y => new Vec4(y, x, 0, y);

		/// <summary>A transpose swizzle property, returns (y,x,x,0)</summary>
		public Vec4 YXX0 => new Vec4(y, x, x, 0);

		/// <summary>A transpose swizzle property, returns (y,x,x,x)</summary>
		public Vec4 YXXX => new Vec4(y, x, x, x);

		/// <summary>A transpose swizzle property, returns (y,x,x,y)</summary>
		public Vec4 YXXY => new Vec4(y, x, x, y);

		/// <summary>A transpose swizzle property, returns (y,x,y,0)</summary>
		public Vec4 YXY0 => new Vec4(y, x, y, 0);

		/// <summary>A transpose swizzle property, returns (y,x,y,x)</summary>
		public Vec4 YXYX => new Vec4(y, x, y, x);

		/// <summary>A transpose swizzle property, returns (y,x,y,y)</summary>
		public Vec4 YXYY => new Vec4(y, x, y, y);

		/// <summary>A transpose swizzle property, returns (y,y,0,0)</summary>
		public Vec4 YY00 => new Vec4(y, y, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,y,0,x)</summary>
		public Vec4 YY0X => new Vec4(y, y, 0, x);

		/// <summary>A transpose swizzle property, returns (y,y,0,y)</summary>
		public Vec4 YY0Y => new Vec4(y, y, 0, y);

		/// <summary>A transpose swizzle property, returns (y,y,x,0)</summary>
		public Vec4 YYX0 => new Vec4(y, y, x, 0);

		/// <summary>A transpose swizzle property, returns (y,y,x,x)</summary>
		public Vec4 YYXX => new Vec4(y, y, x, x);

		/// <summary>A transpose swizzle property, returns (y,y,x,y)</summary>
		public Vec4 YYXY => new Vec4(y, y, x, y);

		/// <summary>A transpose swizzle property, returns (y,y,y,0)</summary>
		public Vec4 YYY0 => new Vec4(y, y, y, 0);

		/// <summary>A transpose swizzle property, returns (y,y,y,x)</summary>
		public Vec4 YYYX => new Vec4(y, y, y, x);

		/// <summary>A transpose swizzle property, returns (y,y,y,y)</summary>
		public Vec4 YYYY => new Vec4(y, y, y, y);

	}
}
