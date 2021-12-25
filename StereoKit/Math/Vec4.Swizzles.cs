
using System;

namespace StereoKit
{
	public partial struct Vec4
	{

		/// <summary>A transpose swizzle property, returns (0,x)</summary>
		public Vec2 OX => new Vec2(0, x);

		/// <summary>A transpose swizzle property, returns (0,y)</summary>
		public Vec2 OY => new Vec2(0, y);

		/// <summary>A transpose swizzle property, returns (0,z)</summary>
		public Vec2 OZ => new Vec2(0, z);

		/// <summary>A transpose swizzle property, returns (0,w)</summary>
		public Vec2 OW => new Vec2(0, w);

		/// <summary>A transpose swizzle property, returns (x,0)</summary>
		public Vec2 X0 => new Vec2(x, 0);

		/// <summary>A transpose swizzle property, returns (x,x)</summary>
		public Vec2 XX => new Vec2(x, x);

		/// <summary>A transpose swizzle property, returns (x,y)</summary>
		public Vec2 XY => new Vec2(x, y);

		/// <summary>A transpose swizzle property, returns (x,z)</summary>
		public Vec2 XZ => new Vec2(x, z);

		/// <summary>A transpose swizzle property, returns (x,w)</summary>
		public Vec2 XW => new Vec2(x, w);

		/// <summary>A transpose swizzle property, returns (y,0)</summary>
		public Vec2 Y0 => new Vec2(y, 0);

		/// <summary>A transpose swizzle property, returns (y,x)</summary>
		public Vec2 YX => new Vec2(y, x);

		/// <summary>A transpose swizzle property, returns (y,y)</summary>
		public Vec2 YY => new Vec2(y, y);

		/// <summary>A transpose swizzle property, returns (y,z)</summary>
		public Vec2 YZ => new Vec2(y, z);

		/// <summary>A transpose swizzle property, returns (y,w)</summary>
		public Vec2 YW => new Vec2(y, w);

		/// <summary>A transpose swizzle property, returns (z,0)</summary>
		public Vec2 Z0 => new Vec2(z, 0);

		/// <summary>A transpose swizzle property, returns (z,x)</summary>
		public Vec2 ZX => new Vec2(z, x);

		/// <summary>A transpose swizzle property, returns (z,y)</summary>
		public Vec2 ZY => new Vec2(z, y);

		/// <summary>A transpose swizzle property, returns (z,z)</summary>
		public Vec2 ZZ => new Vec2(z, z);

		/// <summary>A transpose swizzle property, returns (z,w)</summary>
		public Vec2 ZW => new Vec2(z, w);

		/// <summary>A transpose swizzle property, returns (w,0)</summary>
		public Vec2 W0 => new Vec2(w, 0);

		/// <summary>A transpose swizzle property, returns (w,x)</summary>
		public Vec2 WX => new Vec2(w, x);

		/// <summary>A transpose swizzle property, returns (w,y)</summary>
		public Vec2 WY => new Vec2(w, y);

		/// <summary>A transpose swizzle property, returns (w,z)</summary>
		public Vec2 WZ => new Vec2(w, z);

		/// <summary>A transpose swizzle property, returns (w,w)</summary>
		public Vec2 WW => new Vec2(w, w);

		/// <summary>A transpose swizzle property, returns (0,0,x)</summary>
		public Vec3 O0X => new Vec3(0, 0, x);

		/// <summary>A transpose swizzle property, returns (0,0,y)</summary>
		public Vec3 O0Y => new Vec3(0, 0, y);

		/// <summary>A transpose swizzle property, returns (0,0,z)</summary>
		public Vec3 O0Z => new Vec3(0, 0, z);

		/// <summary>A transpose swizzle property, returns (0,0,w)</summary>
		public Vec3 O0W => new Vec3(0, 0, w);

		/// <summary>A transpose swizzle property, returns (0,x,0)</summary>
		public Vec3 OX0 => new Vec3(0, x, 0);

		/// <summary>A transpose swizzle property, returns (0,x,x)</summary>
		public Vec3 OXX => new Vec3(0, x, x);

		/// <summary>A transpose swizzle property, returns (0,x,y)</summary>
		public Vec3 OXY => new Vec3(0, x, y);

		/// <summary>A transpose swizzle property, returns (0,x,z)</summary>
		public Vec3 OXZ => new Vec3(0, x, z);

		/// <summary>A transpose swizzle property, returns (0,x,w)</summary>
		public Vec3 OXW => new Vec3(0, x, w);

		/// <summary>A transpose swizzle property, returns (0,y,0)</summary>
		public Vec3 OY0 => new Vec3(0, y, 0);

		/// <summary>A transpose swizzle property, returns (0,y,x)</summary>
		public Vec3 OYX => new Vec3(0, y, x);

		/// <summary>A transpose swizzle property, returns (0,y,y)</summary>
		public Vec3 OYY => new Vec3(0, y, y);

		/// <summary>A transpose swizzle property, returns (0,y,z)</summary>
		public Vec3 OYZ => new Vec3(0, y, z);

		/// <summary>A transpose swizzle property, returns (0,y,w)</summary>
		public Vec3 OYW => new Vec3(0, y, w);

		/// <summary>A transpose swizzle property, returns (0,z,0)</summary>
		public Vec3 OZ0 => new Vec3(0, z, 0);

		/// <summary>A transpose swizzle property, returns (0,z,x)</summary>
		public Vec3 OZX => new Vec3(0, z, x);

		/// <summary>A transpose swizzle property, returns (0,z,y)</summary>
		public Vec3 OZY => new Vec3(0, z, y);

		/// <summary>A transpose swizzle property, returns (0,z,z)</summary>
		public Vec3 OZZ => new Vec3(0, z, z);

		/// <summary>A transpose swizzle property, returns (0,z,w)</summary>
		public Vec3 OZW => new Vec3(0, z, w);

		/// <summary>A transpose swizzle property, returns (0,w,0)</summary>
		public Vec3 OW0 => new Vec3(0, w, 0);

		/// <summary>A transpose swizzle property, returns (0,w,x)</summary>
		public Vec3 OWX => new Vec3(0, w, x);

		/// <summary>A transpose swizzle property, returns (0,w,y)</summary>
		public Vec3 OWY => new Vec3(0, w, y);

		/// <summary>A transpose swizzle property, returns (0,w,z)</summary>
		public Vec3 OWZ => new Vec3(0, w, z);

		/// <summary>A transpose swizzle property, returns (0,w,w)</summary>
		public Vec3 OWW => new Vec3(0, w, w);

		/// <summary>A transpose swizzle property, returns (x,0,0)</summary>
		public Vec3 X00 => new Vec3(x, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,0,x)</summary>
		public Vec3 X0X => new Vec3(x, 0, x);

		/// <summary>A transpose swizzle property, returns (x,0,y)</summary>
		public Vec3 X0Y => new Vec3(x, 0, y);

		/// <summary>A transpose swizzle property, returns (x,0,z)</summary>
		public Vec3 X0Z => new Vec3(x, 0, z);

		/// <summary>A transpose swizzle property, returns (x,0,w)</summary>
		public Vec3 X0W => new Vec3(x, 0, w);

		/// <summary>A transpose swizzle property, returns (x,x,0)</summary>
		public Vec3 XX0 => new Vec3(x, x, 0);

		/// <summary>A transpose swizzle property, returns (x,x,x)</summary>
		public Vec3 XXX => new Vec3(x, x, x);

		/// <summary>A transpose swizzle property, returns (x,x,y)</summary>
		public Vec3 XXY => new Vec3(x, x, y);

		/// <summary>A transpose swizzle property, returns (x,x,z)</summary>
		public Vec3 XXZ => new Vec3(x, x, z);

		/// <summary>A transpose swizzle property, returns (x,x,w)</summary>
		public Vec3 XXW => new Vec3(x, x, w);

		/// <summary>A transpose swizzle property, returns (x,y,0)</summary>
		public Vec3 XY0 => new Vec3(x, y, 0);

		/// <summary>A transpose swizzle property, returns (x,y,x)</summary>
		public Vec3 XYX => new Vec3(x, y, x);

		/// <summary>A transpose swizzle property, returns (x,y,y)</summary>
		public Vec3 XYY => new Vec3(x, y, y);

		/// <summary>A transpose swizzle property, returns (x,y,z)</summary>
		public Vec3 XYZ => new Vec3(x, y, z);

		/// <summary>A transpose swizzle property, returns (x,y,w)</summary>
		public Vec3 XYW => new Vec3(x, y, w);

		/// <summary>A transpose swizzle property, returns (x,z,0)</summary>
		public Vec3 XZ0 => new Vec3(x, z, 0);

		/// <summary>A transpose swizzle property, returns (x,z,x)</summary>
		public Vec3 XZX => new Vec3(x, z, x);

		/// <summary>A transpose swizzle property, returns (x,z,y)</summary>
		public Vec3 XZY => new Vec3(x, z, y);

		/// <summary>A transpose swizzle property, returns (x,z,z)</summary>
		public Vec3 XZZ => new Vec3(x, z, z);

		/// <summary>A transpose swizzle property, returns (x,z,w)</summary>
		public Vec3 XZW => new Vec3(x, z, w);

		/// <summary>A transpose swizzle property, returns (x,w,0)</summary>
		public Vec3 XW0 => new Vec3(x, w, 0);

		/// <summary>A transpose swizzle property, returns (x,w,x)</summary>
		public Vec3 XWX => new Vec3(x, w, x);

		/// <summary>A transpose swizzle property, returns (x,w,y)</summary>
		public Vec3 XWY => new Vec3(x, w, y);

		/// <summary>A transpose swizzle property, returns (x,w,z)</summary>
		public Vec3 XWZ => new Vec3(x, w, z);

		/// <summary>A transpose swizzle property, returns (x,w,w)</summary>
		public Vec3 XWW => new Vec3(x, w, w);

		/// <summary>A transpose swizzle property, returns (y,0,0)</summary>
		public Vec3 Y00 => new Vec3(y, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,0,x)</summary>
		public Vec3 Y0X => new Vec3(y, 0, x);

		/// <summary>A transpose swizzle property, returns (y,0,y)</summary>
		public Vec3 Y0Y => new Vec3(y, 0, y);

		/// <summary>A transpose swizzle property, returns (y,0,z)</summary>
		public Vec3 Y0Z => new Vec3(y, 0, z);

		/// <summary>A transpose swizzle property, returns (y,0,w)</summary>
		public Vec3 Y0W => new Vec3(y, 0, w);

		/// <summary>A transpose swizzle property, returns (y,x,0)</summary>
		public Vec3 YX0 => new Vec3(y, x, 0);

		/// <summary>A transpose swizzle property, returns (y,x,x)</summary>
		public Vec3 YXX => new Vec3(y, x, x);

		/// <summary>A transpose swizzle property, returns (y,x,y)</summary>
		public Vec3 YXY => new Vec3(y, x, y);

		/// <summary>A transpose swizzle property, returns (y,x,z)</summary>
		public Vec3 YXZ => new Vec3(y, x, z);

		/// <summary>A transpose swizzle property, returns (y,x,w)</summary>
		public Vec3 YXW => new Vec3(y, x, w);

		/// <summary>A transpose swizzle property, returns (y,y,0)</summary>
		public Vec3 YY0 => new Vec3(y, y, 0);

		/// <summary>A transpose swizzle property, returns (y,y,x)</summary>
		public Vec3 YYX => new Vec3(y, y, x);

		/// <summary>A transpose swizzle property, returns (y,y,y)</summary>
		public Vec3 YYY => new Vec3(y, y, y);

		/// <summary>A transpose swizzle property, returns (y,y,z)</summary>
		public Vec3 YYZ => new Vec3(y, y, z);

		/// <summary>A transpose swizzle property, returns (y,y,w)</summary>
		public Vec3 YYW => new Vec3(y, y, w);

		/// <summary>A transpose swizzle property, returns (y,z,0)</summary>
		public Vec3 YZ0 => new Vec3(y, z, 0);

		/// <summary>A transpose swizzle property, returns (y,z,x)</summary>
		public Vec3 YZX => new Vec3(y, z, x);

		/// <summary>A transpose swizzle property, returns (y,z,y)</summary>
		public Vec3 YZY => new Vec3(y, z, y);

		/// <summary>A transpose swizzle property, returns (y,z,z)</summary>
		public Vec3 YZZ => new Vec3(y, z, z);

		/// <summary>A transpose swizzle property, returns (y,z,w)</summary>
		public Vec3 YZW => new Vec3(y, z, w);

		/// <summary>A transpose swizzle property, returns (y,w,0)</summary>
		public Vec3 YW0 => new Vec3(y, w, 0);

		/// <summary>A transpose swizzle property, returns (y,w,x)</summary>
		public Vec3 YWX => new Vec3(y, w, x);

		/// <summary>A transpose swizzle property, returns (y,w,y)</summary>
		public Vec3 YWY => new Vec3(y, w, y);

		/// <summary>A transpose swizzle property, returns (y,w,z)</summary>
		public Vec3 YWZ => new Vec3(y, w, z);

		/// <summary>A transpose swizzle property, returns (y,w,w)</summary>
		public Vec3 YWW => new Vec3(y, w, w);

		/// <summary>A transpose swizzle property, returns (z,0,0)</summary>
		public Vec3 Z00 => new Vec3(z, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,0,x)</summary>
		public Vec3 Z0X => new Vec3(z, 0, x);

		/// <summary>A transpose swizzle property, returns (z,0,y)</summary>
		public Vec3 Z0Y => new Vec3(z, 0, y);

		/// <summary>A transpose swizzle property, returns (z,0,z)</summary>
		public Vec3 Z0Z => new Vec3(z, 0, z);

		/// <summary>A transpose swizzle property, returns (z,0,w)</summary>
		public Vec3 Z0W => new Vec3(z, 0, w);

		/// <summary>A transpose swizzle property, returns (z,x,0)</summary>
		public Vec3 ZX0 => new Vec3(z, x, 0);

		/// <summary>A transpose swizzle property, returns (z,x,x)</summary>
		public Vec3 ZXX => new Vec3(z, x, x);

		/// <summary>A transpose swizzle property, returns (z,x,y)</summary>
		public Vec3 ZXY => new Vec3(z, x, y);

		/// <summary>A transpose swizzle property, returns (z,x,z)</summary>
		public Vec3 ZXZ => new Vec3(z, x, z);

		/// <summary>A transpose swizzle property, returns (z,x,w)</summary>
		public Vec3 ZXW => new Vec3(z, x, w);

		/// <summary>A transpose swizzle property, returns (z,y,0)</summary>
		public Vec3 ZY0 => new Vec3(z, y, 0);

		/// <summary>A transpose swizzle property, returns (z,y,x)</summary>
		public Vec3 ZYX => new Vec3(z, y, x);

		/// <summary>A transpose swizzle property, returns (z,y,y)</summary>
		public Vec3 ZYY => new Vec3(z, y, y);

		/// <summary>A transpose swizzle property, returns (z,y,z)</summary>
		public Vec3 ZYZ => new Vec3(z, y, z);

		/// <summary>A transpose swizzle property, returns (z,y,w)</summary>
		public Vec3 ZYW => new Vec3(z, y, w);

		/// <summary>A transpose swizzle property, returns (z,z,0)</summary>
		public Vec3 ZZ0 => new Vec3(z, z, 0);

		/// <summary>A transpose swizzle property, returns (z,z,x)</summary>
		public Vec3 ZZX => new Vec3(z, z, x);

		/// <summary>A transpose swizzle property, returns (z,z,y)</summary>
		public Vec3 ZZY => new Vec3(z, z, y);

		/// <summary>A transpose swizzle property, returns (z,z,z)</summary>
		public Vec3 ZZZ => new Vec3(z, z, z);

		/// <summary>A transpose swizzle property, returns (z,z,w)</summary>
		public Vec3 ZZW => new Vec3(z, z, w);

		/// <summary>A transpose swizzle property, returns (z,w,0)</summary>
		public Vec3 ZW0 => new Vec3(z, w, 0);

		/// <summary>A transpose swizzle property, returns (z,w,x)</summary>
		public Vec3 ZWX => new Vec3(z, w, x);

		/// <summary>A transpose swizzle property, returns (z,w,y)</summary>
		public Vec3 ZWY => new Vec3(z, w, y);

		/// <summary>A transpose swizzle property, returns (z,w,z)</summary>
		public Vec3 ZWZ => new Vec3(z, w, z);

		/// <summary>A transpose swizzle property, returns (z,w,w)</summary>
		public Vec3 ZWW => new Vec3(z, w, w);

		/// <summary>A transpose swizzle property, returns (w,0,0)</summary>
		public Vec3 W00 => new Vec3(w, 0, 0);

		/// <summary>A transpose swizzle property, returns (w,0,x)</summary>
		public Vec3 W0X => new Vec3(w, 0, x);

		/// <summary>A transpose swizzle property, returns (w,0,y)</summary>
		public Vec3 W0Y => new Vec3(w, 0, y);

		/// <summary>A transpose swizzle property, returns (w,0,z)</summary>
		public Vec3 W0Z => new Vec3(w, 0, z);

		/// <summary>A transpose swizzle property, returns (w,0,w)</summary>
		public Vec3 W0W => new Vec3(w, 0, w);

		/// <summary>A transpose swizzle property, returns (w,x,0)</summary>
		public Vec3 WX0 => new Vec3(w, x, 0);

		/// <summary>A transpose swizzle property, returns (w,x,x)</summary>
		public Vec3 WXX => new Vec3(w, x, x);

		/// <summary>A transpose swizzle property, returns (w,x,y)</summary>
		public Vec3 WXY => new Vec3(w, x, y);

		/// <summary>A transpose swizzle property, returns (w,x,z)</summary>
		public Vec3 WXZ => new Vec3(w, x, z);

		/// <summary>A transpose swizzle property, returns (w,x,w)</summary>
		public Vec3 WXW => new Vec3(w, x, w);

		/// <summary>A transpose swizzle property, returns (w,y,0)</summary>
		public Vec3 WY0 => new Vec3(w, y, 0);

		/// <summary>A transpose swizzle property, returns (w,y,x)</summary>
		public Vec3 WYX => new Vec3(w, y, x);

		/// <summary>A transpose swizzle property, returns (w,y,y)</summary>
		public Vec3 WYY => new Vec3(w, y, y);

		/// <summary>A transpose swizzle property, returns (w,y,z)</summary>
		public Vec3 WYZ => new Vec3(w, y, z);

		/// <summary>A transpose swizzle property, returns (w,y,w)</summary>
		public Vec3 WYW => new Vec3(w, y, w);

		/// <summary>A transpose swizzle property, returns (w,z,0)</summary>
		public Vec3 WZ0 => new Vec3(w, z, 0);

		/// <summary>A transpose swizzle property, returns (w,z,x)</summary>
		public Vec3 WZX => new Vec3(w, z, x);

		/// <summary>A transpose swizzle property, returns (w,z,y)</summary>
		public Vec3 WZY => new Vec3(w, z, y);

		/// <summary>A transpose swizzle property, returns (w,z,z)</summary>
		public Vec3 WZZ => new Vec3(w, z, z);

		/// <summary>A transpose swizzle property, returns (w,z,w)</summary>
		public Vec3 WZW => new Vec3(w, z, w);

		/// <summary>A transpose swizzle property, returns (w,w,0)</summary>
		public Vec3 WW0 => new Vec3(w, w, 0);

		/// <summary>A transpose swizzle property, returns (w,w,x)</summary>
		public Vec3 WWX => new Vec3(w, w, x);

		/// <summary>A transpose swizzle property, returns (w,w,y)</summary>
		public Vec3 WWY => new Vec3(w, w, y);

		/// <summary>A transpose swizzle property, returns (w,w,z)</summary>
		public Vec3 WWZ => new Vec3(w, w, z);

		/// <summary>A transpose swizzle property, returns (w,w,w)</summary>
		public Vec3 WWW => new Vec3(w, w, w);

		/// <summary>A transpose swizzle property, returns (0,0,0,x)</summary>
		public Vec4 O00X => new Vec4(0, 0, 0, x);

		/// <summary>A transpose swizzle property, returns (0,0,0,y)</summary>
		public Vec4 O00Y => new Vec4(0, 0, 0, y);

		/// <summary>A transpose swizzle property, returns (0,0,0,z)</summary>
		public Vec4 O00Z => new Vec4(0, 0, 0, z);

		/// <summary>A transpose swizzle property, returns (0,0,0,w)</summary>
		public Vec4 O00W => new Vec4(0, 0, 0, w);

		/// <summary>A transpose swizzle property, returns (0,0,x,0)</summary>
		public Vec4 O0X0 => new Vec4(0, 0, x, 0);

		/// <summary>A transpose swizzle property, returns (0,0,x,x)</summary>
		public Vec4 O0XX => new Vec4(0, 0, x, x);

		/// <summary>A transpose swizzle property, returns (0,0,x,y)</summary>
		public Vec4 O0XY => new Vec4(0, 0, x, y);

		/// <summary>A transpose swizzle property, returns (0,0,x,z)</summary>
		public Vec4 O0XZ => new Vec4(0, 0, x, z);

		/// <summary>A transpose swizzle property, returns (0,0,x,w)</summary>
		public Vec4 O0XW => new Vec4(0, 0, x, w);

		/// <summary>A transpose swizzle property, returns (0,0,y,0)</summary>
		public Vec4 O0Y0 => new Vec4(0, 0, y, 0);

		/// <summary>A transpose swizzle property, returns (0,0,y,x)</summary>
		public Vec4 O0YX => new Vec4(0, 0, y, x);

		/// <summary>A transpose swizzle property, returns (0,0,y,y)</summary>
		public Vec4 O0YY => new Vec4(0, 0, y, y);

		/// <summary>A transpose swizzle property, returns (0,0,y,z)</summary>
		public Vec4 O0YZ => new Vec4(0, 0, y, z);

		/// <summary>A transpose swizzle property, returns (0,0,y,w)</summary>
		public Vec4 O0YW => new Vec4(0, 0, y, w);

		/// <summary>A transpose swizzle property, returns (0,0,z,0)</summary>
		public Vec4 O0Z0 => new Vec4(0, 0, z, 0);

		/// <summary>A transpose swizzle property, returns (0,0,z,x)</summary>
		public Vec4 O0ZX => new Vec4(0, 0, z, x);

		/// <summary>A transpose swizzle property, returns (0,0,z,y)</summary>
		public Vec4 O0ZY => new Vec4(0, 0, z, y);

		/// <summary>A transpose swizzle property, returns (0,0,z,z)</summary>
		public Vec4 O0ZZ => new Vec4(0, 0, z, z);

		/// <summary>A transpose swizzle property, returns (0,0,z,w)</summary>
		public Vec4 O0ZW => new Vec4(0, 0, z, w);

		/// <summary>A transpose swizzle property, returns (0,0,w,0)</summary>
		public Vec4 O0W0 => new Vec4(0, 0, w, 0);

		/// <summary>A transpose swizzle property, returns (0,0,w,x)</summary>
		public Vec4 O0WX => new Vec4(0, 0, w, x);

		/// <summary>A transpose swizzle property, returns (0,0,w,y)</summary>
		public Vec4 O0WY => new Vec4(0, 0, w, y);

		/// <summary>A transpose swizzle property, returns (0,0,w,z)</summary>
		public Vec4 O0WZ => new Vec4(0, 0, w, z);

		/// <summary>A transpose swizzle property, returns (0,0,w,w)</summary>
		public Vec4 O0WW => new Vec4(0, 0, w, w);

		/// <summary>A transpose swizzle property, returns (0,x,0,0)</summary>
		public Vec4 OX00 => new Vec4(0, x, 0, 0);

		/// <summary>A transpose swizzle property, returns (0,x,0,x)</summary>
		public Vec4 OX0X => new Vec4(0, x, 0, x);

		/// <summary>A transpose swizzle property, returns (0,x,0,y)</summary>
		public Vec4 OX0Y => new Vec4(0, x, 0, y);

		/// <summary>A transpose swizzle property, returns (0,x,0,z)</summary>
		public Vec4 OX0Z => new Vec4(0, x, 0, z);

		/// <summary>A transpose swizzle property, returns (0,x,0,w)</summary>
		public Vec4 OX0W => new Vec4(0, x, 0, w);

		/// <summary>A transpose swizzle property, returns (0,x,x,0)</summary>
		public Vec4 OXX0 => new Vec4(0, x, x, 0);

		/// <summary>A transpose swizzle property, returns (0,x,x,x)</summary>
		public Vec4 OXXX => new Vec4(0, x, x, x);

		/// <summary>A transpose swizzle property, returns (0,x,x,y)</summary>
		public Vec4 OXXY => new Vec4(0, x, x, y);

		/// <summary>A transpose swizzle property, returns (0,x,x,z)</summary>
		public Vec4 OXXZ => new Vec4(0, x, x, z);

		/// <summary>A transpose swizzle property, returns (0,x,x,w)</summary>
		public Vec4 OXXW => new Vec4(0, x, x, w);

		/// <summary>A transpose swizzle property, returns (0,x,y,0)</summary>
		public Vec4 OXY0 => new Vec4(0, x, y, 0);

		/// <summary>A transpose swizzle property, returns (0,x,y,x)</summary>
		public Vec4 OXYX => new Vec4(0, x, y, x);

		/// <summary>A transpose swizzle property, returns (0,x,y,y)</summary>
		public Vec4 OXYY => new Vec4(0, x, y, y);

		/// <summary>A transpose swizzle property, returns (0,x,y,z)</summary>
		public Vec4 OXYZ => new Vec4(0, x, y, z);

		/// <summary>A transpose swizzle property, returns (0,x,y,w)</summary>
		public Vec4 OXYW => new Vec4(0, x, y, w);

		/// <summary>A transpose swizzle property, returns (0,x,z,0)</summary>
		public Vec4 OXZ0 => new Vec4(0, x, z, 0);

		/// <summary>A transpose swizzle property, returns (0,x,z,x)</summary>
		public Vec4 OXZX => new Vec4(0, x, z, x);

		/// <summary>A transpose swizzle property, returns (0,x,z,y)</summary>
		public Vec4 OXZY => new Vec4(0, x, z, y);

		/// <summary>A transpose swizzle property, returns (0,x,z,z)</summary>
		public Vec4 OXZZ => new Vec4(0, x, z, z);

		/// <summary>A transpose swizzle property, returns (0,x,z,w)</summary>
		public Vec4 OXZW => new Vec4(0, x, z, w);

		/// <summary>A transpose swizzle property, returns (0,x,w,0)</summary>
		public Vec4 OXW0 => new Vec4(0, x, w, 0);

		/// <summary>A transpose swizzle property, returns (0,x,w,x)</summary>
		public Vec4 OXWX => new Vec4(0, x, w, x);

		/// <summary>A transpose swizzle property, returns (0,x,w,y)</summary>
		public Vec4 OXWY => new Vec4(0, x, w, y);

		/// <summary>A transpose swizzle property, returns (0,x,w,z)</summary>
		public Vec4 OXWZ => new Vec4(0, x, w, z);

		/// <summary>A transpose swizzle property, returns (0,x,w,w)</summary>
		public Vec4 OXWW => new Vec4(0, x, w, w);

		/// <summary>A transpose swizzle property, returns (0,y,0,0)</summary>
		public Vec4 OY00 => new Vec4(0, y, 0, 0);

		/// <summary>A transpose swizzle property, returns (0,y,0,x)</summary>
		public Vec4 OY0X => new Vec4(0, y, 0, x);

		/// <summary>A transpose swizzle property, returns (0,y,0,y)</summary>
		public Vec4 OY0Y => new Vec4(0, y, 0, y);

		/// <summary>A transpose swizzle property, returns (0,y,0,z)</summary>
		public Vec4 OY0Z => new Vec4(0, y, 0, z);

		/// <summary>A transpose swizzle property, returns (0,y,0,w)</summary>
		public Vec4 OY0W => new Vec4(0, y, 0, w);

		/// <summary>A transpose swizzle property, returns (0,y,x,0)</summary>
		public Vec4 OYX0 => new Vec4(0, y, x, 0);

		/// <summary>A transpose swizzle property, returns (0,y,x,x)</summary>
		public Vec4 OYXX => new Vec4(0, y, x, x);

		/// <summary>A transpose swizzle property, returns (0,y,x,y)</summary>
		public Vec4 OYXY => new Vec4(0, y, x, y);

		/// <summary>A transpose swizzle property, returns (0,y,x,z)</summary>
		public Vec4 OYXZ => new Vec4(0, y, x, z);

		/// <summary>A transpose swizzle property, returns (0,y,x,w)</summary>
		public Vec4 OYXW => new Vec4(0, y, x, w);

		/// <summary>A transpose swizzle property, returns (0,y,y,0)</summary>
		public Vec4 OYY0 => new Vec4(0, y, y, 0);

		/// <summary>A transpose swizzle property, returns (0,y,y,x)</summary>
		public Vec4 OYYX => new Vec4(0, y, y, x);

		/// <summary>A transpose swizzle property, returns (0,y,y,y)</summary>
		public Vec4 OYYY => new Vec4(0, y, y, y);

		/// <summary>A transpose swizzle property, returns (0,y,y,z)</summary>
		public Vec4 OYYZ => new Vec4(0, y, y, z);

		/// <summary>A transpose swizzle property, returns (0,y,y,w)</summary>
		public Vec4 OYYW => new Vec4(0, y, y, w);

		/// <summary>A transpose swizzle property, returns (0,y,z,0)</summary>
		public Vec4 OYZ0 => new Vec4(0, y, z, 0);

		/// <summary>A transpose swizzle property, returns (0,y,z,x)</summary>
		public Vec4 OYZX => new Vec4(0, y, z, x);

		/// <summary>A transpose swizzle property, returns (0,y,z,y)</summary>
		public Vec4 OYZY => new Vec4(0, y, z, y);

		/// <summary>A transpose swizzle property, returns (0,y,z,z)</summary>
		public Vec4 OYZZ => new Vec4(0, y, z, z);

		/// <summary>A transpose swizzle property, returns (0,y,z,w)</summary>
		public Vec4 OYZW => new Vec4(0, y, z, w);

		/// <summary>A transpose swizzle property, returns (0,y,w,0)</summary>
		public Vec4 OYW0 => new Vec4(0, y, w, 0);

		/// <summary>A transpose swizzle property, returns (0,y,w,x)</summary>
		public Vec4 OYWX => new Vec4(0, y, w, x);

		/// <summary>A transpose swizzle property, returns (0,y,w,y)</summary>
		public Vec4 OYWY => new Vec4(0, y, w, y);

		/// <summary>A transpose swizzle property, returns (0,y,w,z)</summary>
		public Vec4 OYWZ => new Vec4(0, y, w, z);

		/// <summary>A transpose swizzle property, returns (0,y,w,w)</summary>
		public Vec4 OYWW => new Vec4(0, y, w, w);

		/// <summary>A transpose swizzle property, returns (0,z,0,0)</summary>
		public Vec4 OZ00 => new Vec4(0, z, 0, 0);

		/// <summary>A transpose swizzle property, returns (0,z,0,x)</summary>
		public Vec4 OZ0X => new Vec4(0, z, 0, x);

		/// <summary>A transpose swizzle property, returns (0,z,0,y)</summary>
		public Vec4 OZ0Y => new Vec4(0, z, 0, y);

		/// <summary>A transpose swizzle property, returns (0,z,0,z)</summary>
		public Vec4 OZ0Z => new Vec4(0, z, 0, z);

		/// <summary>A transpose swizzle property, returns (0,z,0,w)</summary>
		public Vec4 OZ0W => new Vec4(0, z, 0, w);

		/// <summary>A transpose swizzle property, returns (0,z,x,0)</summary>
		public Vec4 OZX0 => new Vec4(0, z, x, 0);

		/// <summary>A transpose swizzle property, returns (0,z,x,x)</summary>
		public Vec4 OZXX => new Vec4(0, z, x, x);

		/// <summary>A transpose swizzle property, returns (0,z,x,y)</summary>
		public Vec4 OZXY => new Vec4(0, z, x, y);

		/// <summary>A transpose swizzle property, returns (0,z,x,z)</summary>
		public Vec4 OZXZ => new Vec4(0, z, x, z);

		/// <summary>A transpose swizzle property, returns (0,z,x,w)</summary>
		public Vec4 OZXW => new Vec4(0, z, x, w);

		/// <summary>A transpose swizzle property, returns (0,z,y,0)</summary>
		public Vec4 OZY0 => new Vec4(0, z, y, 0);

		/// <summary>A transpose swizzle property, returns (0,z,y,x)</summary>
		public Vec4 OZYX => new Vec4(0, z, y, x);

		/// <summary>A transpose swizzle property, returns (0,z,y,y)</summary>
		public Vec4 OZYY => new Vec4(0, z, y, y);

		/// <summary>A transpose swizzle property, returns (0,z,y,z)</summary>
		public Vec4 OZYZ => new Vec4(0, z, y, z);

		/// <summary>A transpose swizzle property, returns (0,z,y,w)</summary>
		public Vec4 OZYW => new Vec4(0, z, y, w);

		/// <summary>A transpose swizzle property, returns (0,z,z,0)</summary>
		public Vec4 OZZ0 => new Vec4(0, z, z, 0);

		/// <summary>A transpose swizzle property, returns (0,z,z,x)</summary>
		public Vec4 OZZX => new Vec4(0, z, z, x);

		/// <summary>A transpose swizzle property, returns (0,z,z,y)</summary>
		public Vec4 OZZY => new Vec4(0, z, z, y);

		/// <summary>A transpose swizzle property, returns (0,z,z,z)</summary>
		public Vec4 OZZZ => new Vec4(0, z, z, z);

		/// <summary>A transpose swizzle property, returns (0,z,z,w)</summary>
		public Vec4 OZZW => new Vec4(0, z, z, w);

		/// <summary>A transpose swizzle property, returns (0,z,w,0)</summary>
		public Vec4 OZW0 => new Vec4(0, z, w, 0);

		/// <summary>A transpose swizzle property, returns (0,z,w,x)</summary>
		public Vec4 OZWX => new Vec4(0, z, w, x);

		/// <summary>A transpose swizzle property, returns (0,z,w,y)</summary>
		public Vec4 OZWY => new Vec4(0, z, w, y);

		/// <summary>A transpose swizzle property, returns (0,z,w,z)</summary>
		public Vec4 OZWZ => new Vec4(0, z, w, z);

		/// <summary>A transpose swizzle property, returns (0,z,w,w)</summary>
		public Vec4 OZWW => new Vec4(0, z, w, w);

		/// <summary>A transpose swizzle property, returns (0,w,0,0)</summary>
		public Vec4 OW00 => new Vec4(0, w, 0, 0);

		/// <summary>A transpose swizzle property, returns (0,w,0,x)</summary>
		public Vec4 OW0X => new Vec4(0, w, 0, x);

		/// <summary>A transpose swizzle property, returns (0,w,0,y)</summary>
		public Vec4 OW0Y => new Vec4(0, w, 0, y);

		/// <summary>A transpose swizzle property, returns (0,w,0,z)</summary>
		public Vec4 OW0Z => new Vec4(0, w, 0, z);

		/// <summary>A transpose swizzle property, returns (0,w,0,w)</summary>
		public Vec4 OW0W => new Vec4(0, w, 0, w);

		/// <summary>A transpose swizzle property, returns (0,w,x,0)</summary>
		public Vec4 OWX0 => new Vec4(0, w, x, 0);

		/// <summary>A transpose swizzle property, returns (0,w,x,x)</summary>
		public Vec4 OWXX => new Vec4(0, w, x, x);

		/// <summary>A transpose swizzle property, returns (0,w,x,y)</summary>
		public Vec4 OWXY => new Vec4(0, w, x, y);

		/// <summary>A transpose swizzle property, returns (0,w,x,z)</summary>
		public Vec4 OWXZ => new Vec4(0, w, x, z);

		/// <summary>A transpose swizzle property, returns (0,w,x,w)</summary>
		public Vec4 OWXW => new Vec4(0, w, x, w);

		/// <summary>A transpose swizzle property, returns (0,w,y,0)</summary>
		public Vec4 OWY0 => new Vec4(0, w, y, 0);

		/// <summary>A transpose swizzle property, returns (0,w,y,x)</summary>
		public Vec4 OWYX => new Vec4(0, w, y, x);

		/// <summary>A transpose swizzle property, returns (0,w,y,y)</summary>
		public Vec4 OWYY => new Vec4(0, w, y, y);

		/// <summary>A transpose swizzle property, returns (0,w,y,z)</summary>
		public Vec4 OWYZ => new Vec4(0, w, y, z);

		/// <summary>A transpose swizzle property, returns (0,w,y,w)</summary>
		public Vec4 OWYW => new Vec4(0, w, y, w);

		/// <summary>A transpose swizzle property, returns (0,w,z,0)</summary>
		public Vec4 OWZ0 => new Vec4(0, w, z, 0);

		/// <summary>A transpose swizzle property, returns (0,w,z,x)</summary>
		public Vec4 OWZX => new Vec4(0, w, z, x);

		/// <summary>A transpose swizzle property, returns (0,w,z,y)</summary>
		public Vec4 OWZY => new Vec4(0, w, z, y);

		/// <summary>A transpose swizzle property, returns (0,w,z,z)</summary>
		public Vec4 OWZZ => new Vec4(0, w, z, z);

		/// <summary>A transpose swizzle property, returns (0,w,z,w)</summary>
		public Vec4 OWZW => new Vec4(0, w, z, w);

		/// <summary>A transpose swizzle property, returns (0,w,w,0)</summary>
		public Vec4 OWW0 => new Vec4(0, w, w, 0);

		/// <summary>A transpose swizzle property, returns (0,w,w,x)</summary>
		public Vec4 OWWX => new Vec4(0, w, w, x);

		/// <summary>A transpose swizzle property, returns (0,w,w,y)</summary>
		public Vec4 OWWY => new Vec4(0, w, w, y);

		/// <summary>A transpose swizzle property, returns (0,w,w,z)</summary>
		public Vec4 OWWZ => new Vec4(0, w, w, z);

		/// <summary>A transpose swizzle property, returns (0,w,w,w)</summary>
		public Vec4 OWWW => new Vec4(0, w, w, w);

		/// <summary>A transpose swizzle property, returns (x,0,0,0)</summary>
		public Vec4 X000 => new Vec4(x, 0, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,0,0,x)</summary>
		public Vec4 X00X => new Vec4(x, 0, 0, x);

		/// <summary>A transpose swizzle property, returns (x,0,0,y)</summary>
		public Vec4 X00Y => new Vec4(x, 0, 0, y);

		/// <summary>A transpose swizzle property, returns (x,0,0,z)</summary>
		public Vec4 X00Z => new Vec4(x, 0, 0, z);

		/// <summary>A transpose swizzle property, returns (x,0,0,w)</summary>
		public Vec4 X00W => new Vec4(x, 0, 0, w);

		/// <summary>A transpose swizzle property, returns (x,0,x,0)</summary>
		public Vec4 X0X0 => new Vec4(x, 0, x, 0);

		/// <summary>A transpose swizzle property, returns (x,0,x,x)</summary>
		public Vec4 X0XX => new Vec4(x, 0, x, x);

		/// <summary>A transpose swizzle property, returns (x,0,x,y)</summary>
		public Vec4 X0XY => new Vec4(x, 0, x, y);

		/// <summary>A transpose swizzle property, returns (x,0,x,z)</summary>
		public Vec4 X0XZ => new Vec4(x, 0, x, z);

		/// <summary>A transpose swizzle property, returns (x,0,x,w)</summary>
		public Vec4 X0XW => new Vec4(x, 0, x, w);

		/// <summary>A transpose swizzle property, returns (x,0,y,0)</summary>
		public Vec4 X0Y0 => new Vec4(x, 0, y, 0);

		/// <summary>A transpose swizzle property, returns (x,0,y,x)</summary>
		public Vec4 X0YX => new Vec4(x, 0, y, x);

		/// <summary>A transpose swizzle property, returns (x,0,y,y)</summary>
		public Vec4 X0YY => new Vec4(x, 0, y, y);

		/// <summary>A transpose swizzle property, returns (x,0,y,z)</summary>
		public Vec4 X0YZ => new Vec4(x, 0, y, z);

		/// <summary>A transpose swizzle property, returns (x,0,y,w)</summary>
		public Vec4 X0YW => new Vec4(x, 0, y, w);

		/// <summary>A transpose swizzle property, returns (x,0,z,0)</summary>
		public Vec4 X0Z0 => new Vec4(x, 0, z, 0);

		/// <summary>A transpose swizzle property, returns (x,0,z,x)</summary>
		public Vec4 X0ZX => new Vec4(x, 0, z, x);

		/// <summary>A transpose swizzle property, returns (x,0,z,y)</summary>
		public Vec4 X0ZY => new Vec4(x, 0, z, y);

		/// <summary>A transpose swizzle property, returns (x,0,z,z)</summary>
		public Vec4 X0ZZ => new Vec4(x, 0, z, z);

		/// <summary>A transpose swizzle property, returns (x,0,z,w)</summary>
		public Vec4 X0ZW => new Vec4(x, 0, z, w);

		/// <summary>A transpose swizzle property, returns (x,0,w,0)</summary>
		public Vec4 X0W0 => new Vec4(x, 0, w, 0);

		/// <summary>A transpose swizzle property, returns (x,0,w,x)</summary>
		public Vec4 X0WX => new Vec4(x, 0, w, x);

		/// <summary>A transpose swizzle property, returns (x,0,w,y)</summary>
		public Vec4 X0WY => new Vec4(x, 0, w, y);

		/// <summary>A transpose swizzle property, returns (x,0,w,z)</summary>
		public Vec4 X0WZ => new Vec4(x, 0, w, z);

		/// <summary>A transpose swizzle property, returns (x,0,w,w)</summary>
		public Vec4 X0WW => new Vec4(x, 0, w, w);

		/// <summary>A transpose swizzle property, returns (x,x,0,0)</summary>
		public Vec4 XX00 => new Vec4(x, x, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,x,0,x)</summary>
		public Vec4 XX0X => new Vec4(x, x, 0, x);

		/// <summary>A transpose swizzle property, returns (x,x,0,y)</summary>
		public Vec4 XX0Y => new Vec4(x, x, 0, y);

		/// <summary>A transpose swizzle property, returns (x,x,0,z)</summary>
		public Vec4 XX0Z => new Vec4(x, x, 0, z);

		/// <summary>A transpose swizzle property, returns (x,x,0,w)</summary>
		public Vec4 XX0W => new Vec4(x, x, 0, w);

		/// <summary>A transpose swizzle property, returns (x,x,x,0)</summary>
		public Vec4 XXX0 => new Vec4(x, x, x, 0);

		/// <summary>A transpose swizzle property, returns (x,x,x,x)</summary>
		public Vec4 XXXX => new Vec4(x, x, x, x);

		/// <summary>A transpose swizzle property, returns (x,x,x,y)</summary>
		public Vec4 XXXY => new Vec4(x, x, x, y);

		/// <summary>A transpose swizzle property, returns (x,x,x,z)</summary>
		public Vec4 XXXZ => new Vec4(x, x, x, z);

		/// <summary>A transpose swizzle property, returns (x,x,x,w)</summary>
		public Vec4 XXXW => new Vec4(x, x, x, w);

		/// <summary>A transpose swizzle property, returns (x,x,y,0)</summary>
		public Vec4 XXY0 => new Vec4(x, x, y, 0);

		/// <summary>A transpose swizzle property, returns (x,x,y,x)</summary>
		public Vec4 XXYX => new Vec4(x, x, y, x);

		/// <summary>A transpose swizzle property, returns (x,x,y,y)</summary>
		public Vec4 XXYY => new Vec4(x, x, y, y);

		/// <summary>A transpose swizzle property, returns (x,x,y,z)</summary>
		public Vec4 XXYZ => new Vec4(x, x, y, z);

		/// <summary>A transpose swizzle property, returns (x,x,y,w)</summary>
		public Vec4 XXYW => new Vec4(x, x, y, w);

		/// <summary>A transpose swizzle property, returns (x,x,z,0)</summary>
		public Vec4 XXZ0 => new Vec4(x, x, z, 0);

		/// <summary>A transpose swizzle property, returns (x,x,z,x)</summary>
		public Vec4 XXZX => new Vec4(x, x, z, x);

		/// <summary>A transpose swizzle property, returns (x,x,z,y)</summary>
		public Vec4 XXZY => new Vec4(x, x, z, y);

		/// <summary>A transpose swizzle property, returns (x,x,z,z)</summary>
		public Vec4 XXZZ => new Vec4(x, x, z, z);

		/// <summary>A transpose swizzle property, returns (x,x,z,w)</summary>
		public Vec4 XXZW => new Vec4(x, x, z, w);

		/// <summary>A transpose swizzle property, returns (x,x,w,0)</summary>
		public Vec4 XXW0 => new Vec4(x, x, w, 0);

		/// <summary>A transpose swizzle property, returns (x,x,w,x)</summary>
		public Vec4 XXWX => new Vec4(x, x, w, x);

		/// <summary>A transpose swizzle property, returns (x,x,w,y)</summary>
		public Vec4 XXWY => new Vec4(x, x, w, y);

		/// <summary>A transpose swizzle property, returns (x,x,w,z)</summary>
		public Vec4 XXWZ => new Vec4(x, x, w, z);

		/// <summary>A transpose swizzle property, returns (x,x,w,w)</summary>
		public Vec4 XXWW => new Vec4(x, x, w, w);

		/// <summary>A transpose swizzle property, returns (x,y,0,0)</summary>
		public Vec4 XY00 => new Vec4(x, y, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,y,0,x)</summary>
		public Vec4 XY0X => new Vec4(x, y, 0, x);

		/// <summary>A transpose swizzle property, returns (x,y,0,y)</summary>
		public Vec4 XY0Y => new Vec4(x, y, 0, y);

		/// <summary>A transpose swizzle property, returns (x,y,0,z)</summary>
		public Vec4 XY0Z => new Vec4(x, y, 0, z);

		/// <summary>A transpose swizzle property, returns (x,y,0,w)</summary>
		public Vec4 XY0W => new Vec4(x, y, 0, w);

		/// <summary>A transpose swizzle property, returns (x,y,x,0)</summary>
		public Vec4 XYX0 => new Vec4(x, y, x, 0);

		/// <summary>A transpose swizzle property, returns (x,y,x,x)</summary>
		public Vec4 XYXX => new Vec4(x, y, x, x);

		/// <summary>A transpose swizzle property, returns (x,y,x,y)</summary>
		public Vec4 XYXY => new Vec4(x, y, x, y);

		/// <summary>A transpose swizzle property, returns (x,y,x,z)</summary>
		public Vec4 XYXZ => new Vec4(x, y, x, z);

		/// <summary>A transpose swizzle property, returns (x,y,x,w)</summary>
		public Vec4 XYXW => new Vec4(x, y, x, w);

		/// <summary>A transpose swizzle property, returns (x,y,y,0)</summary>
		public Vec4 XYY0 => new Vec4(x, y, y, 0);

		/// <summary>A transpose swizzle property, returns (x,y,y,x)</summary>
		public Vec4 XYYX => new Vec4(x, y, y, x);

		/// <summary>A transpose swizzle property, returns (x,y,y,y)</summary>
		public Vec4 XYYY => new Vec4(x, y, y, y);

		/// <summary>A transpose swizzle property, returns (x,y,y,z)</summary>
		public Vec4 XYYZ => new Vec4(x, y, y, z);

		/// <summary>A transpose swizzle property, returns (x,y,y,w)</summary>
		public Vec4 XYYW => new Vec4(x, y, y, w);

		/// <summary>A transpose swizzle property, returns (x,y,z,0)</summary>
		public Vec4 XYZ0 => new Vec4(x, y, z, 0);

		/// <summary>A transpose swizzle property, returns (x,y,z,x)</summary>
		public Vec4 XYZX => new Vec4(x, y, z, x);

		/// <summary>A transpose swizzle property, returns (x,y,z,y)</summary>
		public Vec4 XYZY => new Vec4(x, y, z, y);

		/// <summary>A transpose swizzle property, returns (x,y,z,z)</summary>
		public Vec4 XYZZ => new Vec4(x, y, z, z);

		/// <summary>A transpose swizzle property, returns (x,y,z,w)</summary>
		public Vec4 XYZW => new Vec4(x, y, z, w);

		/// <summary>A transpose swizzle property, returns (x,y,w,0)</summary>
		public Vec4 XYW0 => new Vec4(x, y, w, 0);

		/// <summary>A transpose swizzle property, returns (x,y,w,x)</summary>
		public Vec4 XYWX => new Vec4(x, y, w, x);

		/// <summary>A transpose swizzle property, returns (x,y,w,y)</summary>
		public Vec4 XYWY => new Vec4(x, y, w, y);

		/// <summary>A transpose swizzle property, returns (x,y,w,z)</summary>
		public Vec4 XYWZ => new Vec4(x, y, w, z);

		/// <summary>A transpose swizzle property, returns (x,y,w,w)</summary>
		public Vec4 XYWW => new Vec4(x, y, w, w);

		/// <summary>A transpose swizzle property, returns (x,z,0,0)</summary>
		public Vec4 XZ00 => new Vec4(x, z, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,z,0,x)</summary>
		public Vec4 XZ0X => new Vec4(x, z, 0, x);

		/// <summary>A transpose swizzle property, returns (x,z,0,y)</summary>
		public Vec4 XZ0Y => new Vec4(x, z, 0, y);

		/// <summary>A transpose swizzle property, returns (x,z,0,z)</summary>
		public Vec4 XZ0Z => new Vec4(x, z, 0, z);

		/// <summary>A transpose swizzle property, returns (x,z,0,w)</summary>
		public Vec4 XZ0W => new Vec4(x, z, 0, w);

		/// <summary>A transpose swizzle property, returns (x,z,x,0)</summary>
		public Vec4 XZX0 => new Vec4(x, z, x, 0);

		/// <summary>A transpose swizzle property, returns (x,z,x,x)</summary>
		public Vec4 XZXX => new Vec4(x, z, x, x);

		/// <summary>A transpose swizzle property, returns (x,z,x,y)</summary>
		public Vec4 XZXY => new Vec4(x, z, x, y);

		/// <summary>A transpose swizzle property, returns (x,z,x,z)</summary>
		public Vec4 XZXZ => new Vec4(x, z, x, z);

		/// <summary>A transpose swizzle property, returns (x,z,x,w)</summary>
		public Vec4 XZXW => new Vec4(x, z, x, w);

		/// <summary>A transpose swizzle property, returns (x,z,y,0)</summary>
		public Vec4 XZY0 => new Vec4(x, z, y, 0);

		/// <summary>A transpose swizzle property, returns (x,z,y,x)</summary>
		public Vec4 XZYX => new Vec4(x, z, y, x);

		/// <summary>A transpose swizzle property, returns (x,z,y,y)</summary>
		public Vec4 XZYY => new Vec4(x, z, y, y);

		/// <summary>A transpose swizzle property, returns (x,z,y,z)</summary>
		public Vec4 XZYZ => new Vec4(x, z, y, z);

		/// <summary>A transpose swizzle property, returns (x,z,y,w)</summary>
		public Vec4 XZYW => new Vec4(x, z, y, w);

		/// <summary>A transpose swizzle property, returns (x,z,z,0)</summary>
		public Vec4 XZZ0 => new Vec4(x, z, z, 0);

		/// <summary>A transpose swizzle property, returns (x,z,z,x)</summary>
		public Vec4 XZZX => new Vec4(x, z, z, x);

		/// <summary>A transpose swizzle property, returns (x,z,z,y)</summary>
		public Vec4 XZZY => new Vec4(x, z, z, y);

		/// <summary>A transpose swizzle property, returns (x,z,z,z)</summary>
		public Vec4 XZZZ => new Vec4(x, z, z, z);

		/// <summary>A transpose swizzle property, returns (x,z,z,w)</summary>
		public Vec4 XZZW => new Vec4(x, z, z, w);

		/// <summary>A transpose swizzle property, returns (x,z,w,0)</summary>
		public Vec4 XZW0 => new Vec4(x, z, w, 0);

		/// <summary>A transpose swizzle property, returns (x,z,w,x)</summary>
		public Vec4 XZWX => new Vec4(x, z, w, x);

		/// <summary>A transpose swizzle property, returns (x,z,w,y)</summary>
		public Vec4 XZWY => new Vec4(x, z, w, y);

		/// <summary>A transpose swizzle property, returns (x,z,w,z)</summary>
		public Vec4 XZWZ => new Vec4(x, z, w, z);

		/// <summary>A transpose swizzle property, returns (x,z,w,w)</summary>
		public Vec4 XZWW => new Vec4(x, z, w, w);

		/// <summary>A transpose swizzle property, returns (x,w,0,0)</summary>
		public Vec4 XW00 => new Vec4(x, w, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,w,0,x)</summary>
		public Vec4 XW0X => new Vec4(x, w, 0, x);

		/// <summary>A transpose swizzle property, returns (x,w,0,y)</summary>
		public Vec4 XW0Y => new Vec4(x, w, 0, y);

		/// <summary>A transpose swizzle property, returns (x,w,0,z)</summary>
		public Vec4 XW0Z => new Vec4(x, w, 0, z);

		/// <summary>A transpose swizzle property, returns (x,w,0,w)</summary>
		public Vec4 XW0W => new Vec4(x, w, 0, w);

		/// <summary>A transpose swizzle property, returns (x,w,x,0)</summary>
		public Vec4 XWX0 => new Vec4(x, w, x, 0);

		/// <summary>A transpose swizzle property, returns (x,w,x,x)</summary>
		public Vec4 XWXX => new Vec4(x, w, x, x);

		/// <summary>A transpose swizzle property, returns (x,w,x,y)</summary>
		public Vec4 XWXY => new Vec4(x, w, x, y);

		/// <summary>A transpose swizzle property, returns (x,w,x,z)</summary>
		public Vec4 XWXZ => new Vec4(x, w, x, z);

		/// <summary>A transpose swizzle property, returns (x,w,x,w)</summary>
		public Vec4 XWXW => new Vec4(x, w, x, w);

		/// <summary>A transpose swizzle property, returns (x,w,y,0)</summary>
		public Vec4 XWY0 => new Vec4(x, w, y, 0);

		/// <summary>A transpose swizzle property, returns (x,w,y,x)</summary>
		public Vec4 XWYX => new Vec4(x, w, y, x);

		/// <summary>A transpose swizzle property, returns (x,w,y,y)</summary>
		public Vec4 XWYY => new Vec4(x, w, y, y);

		/// <summary>A transpose swizzle property, returns (x,w,y,z)</summary>
		public Vec4 XWYZ => new Vec4(x, w, y, z);

		/// <summary>A transpose swizzle property, returns (x,w,y,w)</summary>
		public Vec4 XWYW => new Vec4(x, w, y, w);

		/// <summary>A transpose swizzle property, returns (x,w,z,0)</summary>
		public Vec4 XWZ0 => new Vec4(x, w, z, 0);

		/// <summary>A transpose swizzle property, returns (x,w,z,x)</summary>
		public Vec4 XWZX => new Vec4(x, w, z, x);

		/// <summary>A transpose swizzle property, returns (x,w,z,y)</summary>
		public Vec4 XWZY => new Vec4(x, w, z, y);

		/// <summary>A transpose swizzle property, returns (x,w,z,z)</summary>
		public Vec4 XWZZ => new Vec4(x, w, z, z);

		/// <summary>A transpose swizzle property, returns (x,w,z,w)</summary>
		public Vec4 XWZW => new Vec4(x, w, z, w);

		/// <summary>A transpose swizzle property, returns (x,w,w,0)</summary>
		public Vec4 XWW0 => new Vec4(x, w, w, 0);

		/// <summary>A transpose swizzle property, returns (x,w,w,x)</summary>
		public Vec4 XWWX => new Vec4(x, w, w, x);

		/// <summary>A transpose swizzle property, returns (x,w,w,y)</summary>
		public Vec4 XWWY => new Vec4(x, w, w, y);

		/// <summary>A transpose swizzle property, returns (x,w,w,z)</summary>
		public Vec4 XWWZ => new Vec4(x, w, w, z);

		/// <summary>A transpose swizzle property, returns (x,w,w,w)</summary>
		public Vec4 XWWW => new Vec4(x, w, w, w);

		/// <summary>A transpose swizzle property, returns (y,0,0,0)</summary>
		public Vec4 Y000 => new Vec4(y, 0, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,0,0,x)</summary>
		public Vec4 Y00X => new Vec4(y, 0, 0, x);

		/// <summary>A transpose swizzle property, returns (y,0,0,y)</summary>
		public Vec4 Y00Y => new Vec4(y, 0, 0, y);

		/// <summary>A transpose swizzle property, returns (y,0,0,z)</summary>
		public Vec4 Y00Z => new Vec4(y, 0, 0, z);

		/// <summary>A transpose swizzle property, returns (y,0,0,w)</summary>
		public Vec4 Y00W => new Vec4(y, 0, 0, w);

		/// <summary>A transpose swizzle property, returns (y,0,x,0)</summary>
		public Vec4 Y0X0 => new Vec4(y, 0, x, 0);

		/// <summary>A transpose swizzle property, returns (y,0,x,x)</summary>
		public Vec4 Y0XX => new Vec4(y, 0, x, x);

		/// <summary>A transpose swizzle property, returns (y,0,x,y)</summary>
		public Vec4 Y0XY => new Vec4(y, 0, x, y);

		/// <summary>A transpose swizzle property, returns (y,0,x,z)</summary>
		public Vec4 Y0XZ => new Vec4(y, 0, x, z);

		/// <summary>A transpose swizzle property, returns (y,0,x,w)</summary>
		public Vec4 Y0XW => new Vec4(y, 0, x, w);

		/// <summary>A transpose swizzle property, returns (y,0,y,0)</summary>
		public Vec4 Y0Y0 => new Vec4(y, 0, y, 0);

		/// <summary>A transpose swizzle property, returns (y,0,y,x)</summary>
		public Vec4 Y0YX => new Vec4(y, 0, y, x);

		/// <summary>A transpose swizzle property, returns (y,0,y,y)</summary>
		public Vec4 Y0YY => new Vec4(y, 0, y, y);

		/// <summary>A transpose swizzle property, returns (y,0,y,z)</summary>
		public Vec4 Y0YZ => new Vec4(y, 0, y, z);

		/// <summary>A transpose swizzle property, returns (y,0,y,w)</summary>
		public Vec4 Y0YW => new Vec4(y, 0, y, w);

		/// <summary>A transpose swizzle property, returns (y,0,z,0)</summary>
		public Vec4 Y0Z0 => new Vec4(y, 0, z, 0);

		/// <summary>A transpose swizzle property, returns (y,0,z,x)</summary>
		public Vec4 Y0ZX => new Vec4(y, 0, z, x);

		/// <summary>A transpose swizzle property, returns (y,0,z,y)</summary>
		public Vec4 Y0ZY => new Vec4(y, 0, z, y);

		/// <summary>A transpose swizzle property, returns (y,0,z,z)</summary>
		public Vec4 Y0ZZ => new Vec4(y, 0, z, z);

		/// <summary>A transpose swizzle property, returns (y,0,z,w)</summary>
		public Vec4 Y0ZW => new Vec4(y, 0, z, w);

		/// <summary>A transpose swizzle property, returns (y,0,w,0)</summary>
		public Vec4 Y0W0 => new Vec4(y, 0, w, 0);

		/// <summary>A transpose swizzle property, returns (y,0,w,x)</summary>
		public Vec4 Y0WX => new Vec4(y, 0, w, x);

		/// <summary>A transpose swizzle property, returns (y,0,w,y)</summary>
		public Vec4 Y0WY => new Vec4(y, 0, w, y);

		/// <summary>A transpose swizzle property, returns (y,0,w,z)</summary>
		public Vec4 Y0WZ => new Vec4(y, 0, w, z);

		/// <summary>A transpose swizzle property, returns (y,0,w,w)</summary>
		public Vec4 Y0WW => new Vec4(y, 0, w, w);

		/// <summary>A transpose swizzle property, returns (y,x,0,0)</summary>
		public Vec4 YX00 => new Vec4(y, x, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,x,0,x)</summary>
		public Vec4 YX0X => new Vec4(y, x, 0, x);

		/// <summary>A transpose swizzle property, returns (y,x,0,y)</summary>
		public Vec4 YX0Y => new Vec4(y, x, 0, y);

		/// <summary>A transpose swizzle property, returns (y,x,0,z)</summary>
		public Vec4 YX0Z => new Vec4(y, x, 0, z);

		/// <summary>A transpose swizzle property, returns (y,x,0,w)</summary>
		public Vec4 YX0W => new Vec4(y, x, 0, w);

		/// <summary>A transpose swizzle property, returns (y,x,x,0)</summary>
		public Vec4 YXX0 => new Vec4(y, x, x, 0);

		/// <summary>A transpose swizzle property, returns (y,x,x,x)</summary>
		public Vec4 YXXX => new Vec4(y, x, x, x);

		/// <summary>A transpose swizzle property, returns (y,x,x,y)</summary>
		public Vec4 YXXY => new Vec4(y, x, x, y);

		/// <summary>A transpose swizzle property, returns (y,x,x,z)</summary>
		public Vec4 YXXZ => new Vec4(y, x, x, z);

		/// <summary>A transpose swizzle property, returns (y,x,x,w)</summary>
		public Vec4 YXXW => new Vec4(y, x, x, w);

		/// <summary>A transpose swizzle property, returns (y,x,y,0)</summary>
		public Vec4 YXY0 => new Vec4(y, x, y, 0);

		/// <summary>A transpose swizzle property, returns (y,x,y,x)</summary>
		public Vec4 YXYX => new Vec4(y, x, y, x);

		/// <summary>A transpose swizzle property, returns (y,x,y,y)</summary>
		public Vec4 YXYY => new Vec4(y, x, y, y);

		/// <summary>A transpose swizzle property, returns (y,x,y,z)</summary>
		public Vec4 YXYZ => new Vec4(y, x, y, z);

		/// <summary>A transpose swizzle property, returns (y,x,y,w)</summary>
		public Vec4 YXYW => new Vec4(y, x, y, w);

		/// <summary>A transpose swizzle property, returns (y,x,z,0)</summary>
		public Vec4 YXZ0 => new Vec4(y, x, z, 0);

		/// <summary>A transpose swizzle property, returns (y,x,z,x)</summary>
		public Vec4 YXZX => new Vec4(y, x, z, x);

		/// <summary>A transpose swizzle property, returns (y,x,z,y)</summary>
		public Vec4 YXZY => new Vec4(y, x, z, y);

		/// <summary>A transpose swizzle property, returns (y,x,z,z)</summary>
		public Vec4 YXZZ => new Vec4(y, x, z, z);

		/// <summary>A transpose swizzle property, returns (y,x,z,w)</summary>
		public Vec4 YXZW => new Vec4(y, x, z, w);

		/// <summary>A transpose swizzle property, returns (y,x,w,0)</summary>
		public Vec4 YXW0 => new Vec4(y, x, w, 0);

		/// <summary>A transpose swizzle property, returns (y,x,w,x)</summary>
		public Vec4 YXWX => new Vec4(y, x, w, x);

		/// <summary>A transpose swizzle property, returns (y,x,w,y)</summary>
		public Vec4 YXWY => new Vec4(y, x, w, y);

		/// <summary>A transpose swizzle property, returns (y,x,w,z)</summary>
		public Vec4 YXWZ => new Vec4(y, x, w, z);

		/// <summary>A transpose swizzle property, returns (y,x,w,w)</summary>
		public Vec4 YXWW => new Vec4(y, x, w, w);

		/// <summary>A transpose swizzle property, returns (y,y,0,0)</summary>
		public Vec4 YY00 => new Vec4(y, y, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,y,0,x)</summary>
		public Vec4 YY0X => new Vec4(y, y, 0, x);

		/// <summary>A transpose swizzle property, returns (y,y,0,y)</summary>
		public Vec4 YY0Y => new Vec4(y, y, 0, y);

		/// <summary>A transpose swizzle property, returns (y,y,0,z)</summary>
		public Vec4 YY0Z => new Vec4(y, y, 0, z);

		/// <summary>A transpose swizzle property, returns (y,y,0,w)</summary>
		public Vec4 YY0W => new Vec4(y, y, 0, w);

		/// <summary>A transpose swizzle property, returns (y,y,x,0)</summary>
		public Vec4 YYX0 => new Vec4(y, y, x, 0);

		/// <summary>A transpose swizzle property, returns (y,y,x,x)</summary>
		public Vec4 YYXX => new Vec4(y, y, x, x);

		/// <summary>A transpose swizzle property, returns (y,y,x,y)</summary>
		public Vec4 YYXY => new Vec4(y, y, x, y);

		/// <summary>A transpose swizzle property, returns (y,y,x,z)</summary>
		public Vec4 YYXZ => new Vec4(y, y, x, z);

		/// <summary>A transpose swizzle property, returns (y,y,x,w)</summary>
		public Vec4 YYXW => new Vec4(y, y, x, w);

		/// <summary>A transpose swizzle property, returns (y,y,y,0)</summary>
		public Vec4 YYY0 => new Vec4(y, y, y, 0);

		/// <summary>A transpose swizzle property, returns (y,y,y,x)</summary>
		public Vec4 YYYX => new Vec4(y, y, y, x);

		/// <summary>A transpose swizzle property, returns (y,y,y,y)</summary>
		public Vec4 YYYY => new Vec4(y, y, y, y);

		/// <summary>A transpose swizzle property, returns (y,y,y,z)</summary>
		public Vec4 YYYZ => new Vec4(y, y, y, z);

		/// <summary>A transpose swizzle property, returns (y,y,y,w)</summary>
		public Vec4 YYYW => new Vec4(y, y, y, w);

		/// <summary>A transpose swizzle property, returns (y,y,z,0)</summary>
		public Vec4 YYZ0 => new Vec4(y, y, z, 0);

		/// <summary>A transpose swizzle property, returns (y,y,z,x)</summary>
		public Vec4 YYZX => new Vec4(y, y, z, x);

		/// <summary>A transpose swizzle property, returns (y,y,z,y)</summary>
		public Vec4 YYZY => new Vec4(y, y, z, y);

		/// <summary>A transpose swizzle property, returns (y,y,z,z)</summary>
		public Vec4 YYZZ => new Vec4(y, y, z, z);

		/// <summary>A transpose swizzle property, returns (y,y,z,w)</summary>
		public Vec4 YYZW => new Vec4(y, y, z, w);

		/// <summary>A transpose swizzle property, returns (y,y,w,0)</summary>
		public Vec4 YYW0 => new Vec4(y, y, w, 0);

		/// <summary>A transpose swizzle property, returns (y,y,w,x)</summary>
		public Vec4 YYWX => new Vec4(y, y, w, x);

		/// <summary>A transpose swizzle property, returns (y,y,w,y)</summary>
		public Vec4 YYWY => new Vec4(y, y, w, y);

		/// <summary>A transpose swizzle property, returns (y,y,w,z)</summary>
		public Vec4 YYWZ => new Vec4(y, y, w, z);

		/// <summary>A transpose swizzle property, returns (y,y,w,w)</summary>
		public Vec4 YYWW => new Vec4(y, y, w, w);

		/// <summary>A transpose swizzle property, returns (y,z,0,0)</summary>
		public Vec4 YZ00 => new Vec4(y, z, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,z,0,x)</summary>
		public Vec4 YZ0X => new Vec4(y, z, 0, x);

		/// <summary>A transpose swizzle property, returns (y,z,0,y)</summary>
		public Vec4 YZ0Y => new Vec4(y, z, 0, y);

		/// <summary>A transpose swizzle property, returns (y,z,0,z)</summary>
		public Vec4 YZ0Z => new Vec4(y, z, 0, z);

		/// <summary>A transpose swizzle property, returns (y,z,0,w)</summary>
		public Vec4 YZ0W => new Vec4(y, z, 0, w);

		/// <summary>A transpose swizzle property, returns (y,z,x,0)</summary>
		public Vec4 YZX0 => new Vec4(y, z, x, 0);

		/// <summary>A transpose swizzle property, returns (y,z,x,x)</summary>
		public Vec4 YZXX => new Vec4(y, z, x, x);

		/// <summary>A transpose swizzle property, returns (y,z,x,y)</summary>
		public Vec4 YZXY => new Vec4(y, z, x, y);

		/// <summary>A transpose swizzle property, returns (y,z,x,z)</summary>
		public Vec4 YZXZ => new Vec4(y, z, x, z);

		/// <summary>A transpose swizzle property, returns (y,z,x,w)</summary>
		public Vec4 YZXW => new Vec4(y, z, x, w);

		/// <summary>A transpose swizzle property, returns (y,z,y,0)</summary>
		public Vec4 YZY0 => new Vec4(y, z, y, 0);

		/// <summary>A transpose swizzle property, returns (y,z,y,x)</summary>
		public Vec4 YZYX => new Vec4(y, z, y, x);

		/// <summary>A transpose swizzle property, returns (y,z,y,y)</summary>
		public Vec4 YZYY => new Vec4(y, z, y, y);

		/// <summary>A transpose swizzle property, returns (y,z,y,z)</summary>
		public Vec4 YZYZ => new Vec4(y, z, y, z);

		/// <summary>A transpose swizzle property, returns (y,z,y,w)</summary>
		public Vec4 YZYW => new Vec4(y, z, y, w);

		/// <summary>A transpose swizzle property, returns (y,z,z,0)</summary>
		public Vec4 YZZ0 => new Vec4(y, z, z, 0);

		/// <summary>A transpose swizzle property, returns (y,z,z,x)</summary>
		public Vec4 YZZX => new Vec4(y, z, z, x);

		/// <summary>A transpose swizzle property, returns (y,z,z,y)</summary>
		public Vec4 YZZY => new Vec4(y, z, z, y);

		/// <summary>A transpose swizzle property, returns (y,z,z,z)</summary>
		public Vec4 YZZZ => new Vec4(y, z, z, z);

		/// <summary>A transpose swizzle property, returns (y,z,z,w)</summary>
		public Vec4 YZZW => new Vec4(y, z, z, w);

		/// <summary>A transpose swizzle property, returns (y,z,w,0)</summary>
		public Vec4 YZW0 => new Vec4(y, z, w, 0);

		/// <summary>A transpose swizzle property, returns (y,z,w,x)</summary>
		public Vec4 YZWX => new Vec4(y, z, w, x);

		/// <summary>A transpose swizzle property, returns (y,z,w,y)</summary>
		public Vec4 YZWY => new Vec4(y, z, w, y);

		/// <summary>A transpose swizzle property, returns (y,z,w,z)</summary>
		public Vec4 YZWZ => new Vec4(y, z, w, z);

		/// <summary>A transpose swizzle property, returns (y,z,w,w)</summary>
		public Vec4 YZWW => new Vec4(y, z, w, w);

		/// <summary>A transpose swizzle property, returns (y,w,0,0)</summary>
		public Vec4 YW00 => new Vec4(y, w, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,w,0,x)</summary>
		public Vec4 YW0X => new Vec4(y, w, 0, x);

		/// <summary>A transpose swizzle property, returns (y,w,0,y)</summary>
		public Vec4 YW0Y => new Vec4(y, w, 0, y);

		/// <summary>A transpose swizzle property, returns (y,w,0,z)</summary>
		public Vec4 YW0Z => new Vec4(y, w, 0, z);

		/// <summary>A transpose swizzle property, returns (y,w,0,w)</summary>
		public Vec4 YW0W => new Vec4(y, w, 0, w);

		/// <summary>A transpose swizzle property, returns (y,w,x,0)</summary>
		public Vec4 YWX0 => new Vec4(y, w, x, 0);

		/// <summary>A transpose swizzle property, returns (y,w,x,x)</summary>
		public Vec4 YWXX => new Vec4(y, w, x, x);

		/// <summary>A transpose swizzle property, returns (y,w,x,y)</summary>
		public Vec4 YWXY => new Vec4(y, w, x, y);

		/// <summary>A transpose swizzle property, returns (y,w,x,z)</summary>
		public Vec4 YWXZ => new Vec4(y, w, x, z);

		/// <summary>A transpose swizzle property, returns (y,w,x,w)</summary>
		public Vec4 YWXW => new Vec4(y, w, x, w);

		/// <summary>A transpose swizzle property, returns (y,w,y,0)</summary>
		public Vec4 YWY0 => new Vec4(y, w, y, 0);

		/// <summary>A transpose swizzle property, returns (y,w,y,x)</summary>
		public Vec4 YWYX => new Vec4(y, w, y, x);

		/// <summary>A transpose swizzle property, returns (y,w,y,y)</summary>
		public Vec4 YWYY => new Vec4(y, w, y, y);

		/// <summary>A transpose swizzle property, returns (y,w,y,z)</summary>
		public Vec4 YWYZ => new Vec4(y, w, y, z);

		/// <summary>A transpose swizzle property, returns (y,w,y,w)</summary>
		public Vec4 YWYW => new Vec4(y, w, y, w);

		/// <summary>A transpose swizzle property, returns (y,w,z,0)</summary>
		public Vec4 YWZ0 => new Vec4(y, w, z, 0);

		/// <summary>A transpose swizzle property, returns (y,w,z,x)</summary>
		public Vec4 YWZX => new Vec4(y, w, z, x);

		/// <summary>A transpose swizzle property, returns (y,w,z,y)</summary>
		public Vec4 YWZY => new Vec4(y, w, z, y);

		/// <summary>A transpose swizzle property, returns (y,w,z,z)</summary>
		public Vec4 YWZZ => new Vec4(y, w, z, z);

		/// <summary>A transpose swizzle property, returns (y,w,z,w)</summary>
		public Vec4 YWZW => new Vec4(y, w, z, w);

		/// <summary>A transpose swizzle property, returns (y,w,w,0)</summary>
		public Vec4 YWW0 => new Vec4(y, w, w, 0);

		/// <summary>A transpose swizzle property, returns (y,w,w,x)</summary>
		public Vec4 YWWX => new Vec4(y, w, w, x);

		/// <summary>A transpose swizzle property, returns (y,w,w,y)</summary>
		public Vec4 YWWY => new Vec4(y, w, w, y);

		/// <summary>A transpose swizzle property, returns (y,w,w,z)</summary>
		public Vec4 YWWZ => new Vec4(y, w, w, z);

		/// <summary>A transpose swizzle property, returns (y,w,w,w)</summary>
		public Vec4 YWWW => new Vec4(y, w, w, w);

		/// <summary>A transpose swizzle property, returns (z,0,0,0)</summary>
		public Vec4 Z000 => new Vec4(z, 0, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,0,0,x)</summary>
		public Vec4 Z00X => new Vec4(z, 0, 0, x);

		/// <summary>A transpose swizzle property, returns (z,0,0,y)</summary>
		public Vec4 Z00Y => new Vec4(z, 0, 0, y);

		/// <summary>A transpose swizzle property, returns (z,0,0,z)</summary>
		public Vec4 Z00Z => new Vec4(z, 0, 0, z);

		/// <summary>A transpose swizzle property, returns (z,0,0,w)</summary>
		public Vec4 Z00W => new Vec4(z, 0, 0, w);

		/// <summary>A transpose swizzle property, returns (z,0,x,0)</summary>
		public Vec4 Z0X0 => new Vec4(z, 0, x, 0);

		/// <summary>A transpose swizzle property, returns (z,0,x,x)</summary>
		public Vec4 Z0XX => new Vec4(z, 0, x, x);

		/// <summary>A transpose swizzle property, returns (z,0,x,y)</summary>
		public Vec4 Z0XY => new Vec4(z, 0, x, y);

		/// <summary>A transpose swizzle property, returns (z,0,x,z)</summary>
		public Vec4 Z0XZ => new Vec4(z, 0, x, z);

		/// <summary>A transpose swizzle property, returns (z,0,x,w)</summary>
		public Vec4 Z0XW => new Vec4(z, 0, x, w);

		/// <summary>A transpose swizzle property, returns (z,0,y,0)</summary>
		public Vec4 Z0Y0 => new Vec4(z, 0, y, 0);

		/// <summary>A transpose swizzle property, returns (z,0,y,x)</summary>
		public Vec4 Z0YX => new Vec4(z, 0, y, x);

		/// <summary>A transpose swizzle property, returns (z,0,y,y)</summary>
		public Vec4 Z0YY => new Vec4(z, 0, y, y);

		/// <summary>A transpose swizzle property, returns (z,0,y,z)</summary>
		public Vec4 Z0YZ => new Vec4(z, 0, y, z);

		/// <summary>A transpose swizzle property, returns (z,0,y,w)</summary>
		public Vec4 Z0YW => new Vec4(z, 0, y, w);

		/// <summary>A transpose swizzle property, returns (z,0,z,0)</summary>
		public Vec4 Z0Z0 => new Vec4(z, 0, z, 0);

		/// <summary>A transpose swizzle property, returns (z,0,z,x)</summary>
		public Vec4 Z0ZX => new Vec4(z, 0, z, x);

		/// <summary>A transpose swizzle property, returns (z,0,z,y)</summary>
		public Vec4 Z0ZY => new Vec4(z, 0, z, y);

		/// <summary>A transpose swizzle property, returns (z,0,z,z)</summary>
		public Vec4 Z0ZZ => new Vec4(z, 0, z, z);

		/// <summary>A transpose swizzle property, returns (z,0,z,w)</summary>
		public Vec4 Z0ZW => new Vec4(z, 0, z, w);

		/// <summary>A transpose swizzle property, returns (z,0,w,0)</summary>
		public Vec4 Z0W0 => new Vec4(z, 0, w, 0);

		/// <summary>A transpose swizzle property, returns (z,0,w,x)</summary>
		public Vec4 Z0WX => new Vec4(z, 0, w, x);

		/// <summary>A transpose swizzle property, returns (z,0,w,y)</summary>
		public Vec4 Z0WY => new Vec4(z, 0, w, y);

		/// <summary>A transpose swizzle property, returns (z,0,w,z)</summary>
		public Vec4 Z0WZ => new Vec4(z, 0, w, z);

		/// <summary>A transpose swizzle property, returns (z,0,w,w)</summary>
		public Vec4 Z0WW => new Vec4(z, 0, w, w);

		/// <summary>A transpose swizzle property, returns (z,x,0,0)</summary>
		public Vec4 ZX00 => new Vec4(z, x, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,x,0,x)</summary>
		public Vec4 ZX0X => new Vec4(z, x, 0, x);

		/// <summary>A transpose swizzle property, returns (z,x,0,y)</summary>
		public Vec4 ZX0Y => new Vec4(z, x, 0, y);

		/// <summary>A transpose swizzle property, returns (z,x,0,z)</summary>
		public Vec4 ZX0Z => new Vec4(z, x, 0, z);

		/// <summary>A transpose swizzle property, returns (z,x,0,w)</summary>
		public Vec4 ZX0W => new Vec4(z, x, 0, w);

		/// <summary>A transpose swizzle property, returns (z,x,x,0)</summary>
		public Vec4 ZXX0 => new Vec4(z, x, x, 0);

		/// <summary>A transpose swizzle property, returns (z,x,x,x)</summary>
		public Vec4 ZXXX => new Vec4(z, x, x, x);

		/// <summary>A transpose swizzle property, returns (z,x,x,y)</summary>
		public Vec4 ZXXY => new Vec4(z, x, x, y);

		/// <summary>A transpose swizzle property, returns (z,x,x,z)</summary>
		public Vec4 ZXXZ => new Vec4(z, x, x, z);

		/// <summary>A transpose swizzle property, returns (z,x,x,w)</summary>
		public Vec4 ZXXW => new Vec4(z, x, x, w);

		/// <summary>A transpose swizzle property, returns (z,x,y,0)</summary>
		public Vec4 ZXY0 => new Vec4(z, x, y, 0);

		/// <summary>A transpose swizzle property, returns (z,x,y,x)</summary>
		public Vec4 ZXYX => new Vec4(z, x, y, x);

		/// <summary>A transpose swizzle property, returns (z,x,y,y)</summary>
		public Vec4 ZXYY => new Vec4(z, x, y, y);

		/// <summary>A transpose swizzle property, returns (z,x,y,z)</summary>
		public Vec4 ZXYZ => new Vec4(z, x, y, z);

		/// <summary>A transpose swizzle property, returns (z,x,y,w)</summary>
		public Vec4 ZXYW => new Vec4(z, x, y, w);

		/// <summary>A transpose swizzle property, returns (z,x,z,0)</summary>
		public Vec4 ZXZ0 => new Vec4(z, x, z, 0);

		/// <summary>A transpose swizzle property, returns (z,x,z,x)</summary>
		public Vec4 ZXZX => new Vec4(z, x, z, x);

		/// <summary>A transpose swizzle property, returns (z,x,z,y)</summary>
		public Vec4 ZXZY => new Vec4(z, x, z, y);

		/// <summary>A transpose swizzle property, returns (z,x,z,z)</summary>
		public Vec4 ZXZZ => new Vec4(z, x, z, z);

		/// <summary>A transpose swizzle property, returns (z,x,z,w)</summary>
		public Vec4 ZXZW => new Vec4(z, x, z, w);

		/// <summary>A transpose swizzle property, returns (z,x,w,0)</summary>
		public Vec4 ZXW0 => new Vec4(z, x, w, 0);

		/// <summary>A transpose swizzle property, returns (z,x,w,x)</summary>
		public Vec4 ZXWX => new Vec4(z, x, w, x);

		/// <summary>A transpose swizzle property, returns (z,x,w,y)</summary>
		public Vec4 ZXWY => new Vec4(z, x, w, y);

		/// <summary>A transpose swizzle property, returns (z,x,w,z)</summary>
		public Vec4 ZXWZ => new Vec4(z, x, w, z);

		/// <summary>A transpose swizzle property, returns (z,x,w,w)</summary>
		public Vec4 ZXWW => new Vec4(z, x, w, w);

		/// <summary>A transpose swizzle property, returns (z,y,0,0)</summary>
		public Vec4 ZY00 => new Vec4(z, y, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,y,0,x)</summary>
		public Vec4 ZY0X => new Vec4(z, y, 0, x);

		/// <summary>A transpose swizzle property, returns (z,y,0,y)</summary>
		public Vec4 ZY0Y => new Vec4(z, y, 0, y);

		/// <summary>A transpose swizzle property, returns (z,y,0,z)</summary>
		public Vec4 ZY0Z => new Vec4(z, y, 0, z);

		/// <summary>A transpose swizzle property, returns (z,y,0,w)</summary>
		public Vec4 ZY0W => new Vec4(z, y, 0, w);

		/// <summary>A transpose swizzle property, returns (z,y,x,0)</summary>
		public Vec4 ZYX0 => new Vec4(z, y, x, 0);

		/// <summary>A transpose swizzle property, returns (z,y,x,x)</summary>
		public Vec4 ZYXX => new Vec4(z, y, x, x);

		/// <summary>A transpose swizzle property, returns (z,y,x,y)</summary>
		public Vec4 ZYXY => new Vec4(z, y, x, y);

		/// <summary>A transpose swizzle property, returns (z,y,x,z)</summary>
		public Vec4 ZYXZ => new Vec4(z, y, x, z);

		/// <summary>A transpose swizzle property, returns (z,y,x,w)</summary>
		public Vec4 ZYXW => new Vec4(z, y, x, w);

		/// <summary>A transpose swizzle property, returns (z,y,y,0)</summary>
		public Vec4 ZYY0 => new Vec4(z, y, y, 0);

		/// <summary>A transpose swizzle property, returns (z,y,y,x)</summary>
		public Vec4 ZYYX => new Vec4(z, y, y, x);

		/// <summary>A transpose swizzle property, returns (z,y,y,y)</summary>
		public Vec4 ZYYY => new Vec4(z, y, y, y);

		/// <summary>A transpose swizzle property, returns (z,y,y,z)</summary>
		public Vec4 ZYYZ => new Vec4(z, y, y, z);

		/// <summary>A transpose swizzle property, returns (z,y,y,w)</summary>
		public Vec4 ZYYW => new Vec4(z, y, y, w);

		/// <summary>A transpose swizzle property, returns (z,y,z,0)</summary>
		public Vec4 ZYZ0 => new Vec4(z, y, z, 0);

		/// <summary>A transpose swizzle property, returns (z,y,z,x)</summary>
		public Vec4 ZYZX => new Vec4(z, y, z, x);

		/// <summary>A transpose swizzle property, returns (z,y,z,y)</summary>
		public Vec4 ZYZY => new Vec4(z, y, z, y);

		/// <summary>A transpose swizzle property, returns (z,y,z,z)</summary>
		public Vec4 ZYZZ => new Vec4(z, y, z, z);

		/// <summary>A transpose swizzle property, returns (z,y,z,w)</summary>
		public Vec4 ZYZW => new Vec4(z, y, z, w);

		/// <summary>A transpose swizzle property, returns (z,y,w,0)</summary>
		public Vec4 ZYW0 => new Vec4(z, y, w, 0);

		/// <summary>A transpose swizzle property, returns (z,y,w,x)</summary>
		public Vec4 ZYWX => new Vec4(z, y, w, x);

		/// <summary>A transpose swizzle property, returns (z,y,w,y)</summary>
		public Vec4 ZYWY => new Vec4(z, y, w, y);

		/// <summary>A transpose swizzle property, returns (z,y,w,z)</summary>
		public Vec4 ZYWZ => new Vec4(z, y, w, z);

		/// <summary>A transpose swizzle property, returns (z,y,w,w)</summary>
		public Vec4 ZYWW => new Vec4(z, y, w, w);

		/// <summary>A transpose swizzle property, returns (z,z,0,0)</summary>
		public Vec4 ZZ00 => new Vec4(z, z, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,z,0,x)</summary>
		public Vec4 ZZ0X => new Vec4(z, z, 0, x);

		/// <summary>A transpose swizzle property, returns (z,z,0,y)</summary>
		public Vec4 ZZ0Y => new Vec4(z, z, 0, y);

		/// <summary>A transpose swizzle property, returns (z,z,0,z)</summary>
		public Vec4 ZZ0Z => new Vec4(z, z, 0, z);

		/// <summary>A transpose swizzle property, returns (z,z,0,w)</summary>
		public Vec4 ZZ0W => new Vec4(z, z, 0, w);

		/// <summary>A transpose swizzle property, returns (z,z,x,0)</summary>
		public Vec4 ZZX0 => new Vec4(z, z, x, 0);

		/// <summary>A transpose swizzle property, returns (z,z,x,x)</summary>
		public Vec4 ZZXX => new Vec4(z, z, x, x);

		/// <summary>A transpose swizzle property, returns (z,z,x,y)</summary>
		public Vec4 ZZXY => new Vec4(z, z, x, y);

		/// <summary>A transpose swizzle property, returns (z,z,x,z)</summary>
		public Vec4 ZZXZ => new Vec4(z, z, x, z);

		/// <summary>A transpose swizzle property, returns (z,z,x,w)</summary>
		public Vec4 ZZXW => new Vec4(z, z, x, w);

		/// <summary>A transpose swizzle property, returns (z,z,y,0)</summary>
		public Vec4 ZZY0 => new Vec4(z, z, y, 0);

		/// <summary>A transpose swizzle property, returns (z,z,y,x)</summary>
		public Vec4 ZZYX => new Vec4(z, z, y, x);

		/// <summary>A transpose swizzle property, returns (z,z,y,y)</summary>
		public Vec4 ZZYY => new Vec4(z, z, y, y);

		/// <summary>A transpose swizzle property, returns (z,z,y,z)</summary>
		public Vec4 ZZYZ => new Vec4(z, z, y, z);

		/// <summary>A transpose swizzle property, returns (z,z,y,w)</summary>
		public Vec4 ZZYW => new Vec4(z, z, y, w);

		/// <summary>A transpose swizzle property, returns (z,z,z,0)</summary>
		public Vec4 ZZZ0 => new Vec4(z, z, z, 0);

		/// <summary>A transpose swizzle property, returns (z,z,z,x)</summary>
		public Vec4 ZZZX => new Vec4(z, z, z, x);

		/// <summary>A transpose swizzle property, returns (z,z,z,y)</summary>
		public Vec4 ZZZY => new Vec4(z, z, z, y);

		/// <summary>A transpose swizzle property, returns (z,z,z,z)</summary>
		public Vec4 ZZZZ => new Vec4(z, z, z, z);

		/// <summary>A transpose swizzle property, returns (z,z,z,w)</summary>
		public Vec4 ZZZW => new Vec4(z, z, z, w);

		/// <summary>A transpose swizzle property, returns (z,z,w,0)</summary>
		public Vec4 ZZW0 => new Vec4(z, z, w, 0);

		/// <summary>A transpose swizzle property, returns (z,z,w,x)</summary>
		public Vec4 ZZWX => new Vec4(z, z, w, x);

		/// <summary>A transpose swizzle property, returns (z,z,w,y)</summary>
		public Vec4 ZZWY => new Vec4(z, z, w, y);

		/// <summary>A transpose swizzle property, returns (z,z,w,z)</summary>
		public Vec4 ZZWZ => new Vec4(z, z, w, z);

		/// <summary>A transpose swizzle property, returns (z,z,w,w)</summary>
		public Vec4 ZZWW => new Vec4(z, z, w, w);

		/// <summary>A transpose swizzle property, returns (z,w,0,0)</summary>
		public Vec4 ZW00 => new Vec4(z, w, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,w,0,x)</summary>
		public Vec4 ZW0X => new Vec4(z, w, 0, x);

		/// <summary>A transpose swizzle property, returns (z,w,0,y)</summary>
		public Vec4 ZW0Y => new Vec4(z, w, 0, y);

		/// <summary>A transpose swizzle property, returns (z,w,0,z)</summary>
		public Vec4 ZW0Z => new Vec4(z, w, 0, z);

		/// <summary>A transpose swizzle property, returns (z,w,0,w)</summary>
		public Vec4 ZW0W => new Vec4(z, w, 0, w);

		/// <summary>A transpose swizzle property, returns (z,w,x,0)</summary>
		public Vec4 ZWX0 => new Vec4(z, w, x, 0);

		/// <summary>A transpose swizzle property, returns (z,w,x,x)</summary>
		public Vec4 ZWXX => new Vec4(z, w, x, x);

		/// <summary>A transpose swizzle property, returns (z,w,x,y)</summary>
		public Vec4 ZWXY => new Vec4(z, w, x, y);

		/// <summary>A transpose swizzle property, returns (z,w,x,z)</summary>
		public Vec4 ZWXZ => new Vec4(z, w, x, z);

		/// <summary>A transpose swizzle property, returns (z,w,x,w)</summary>
		public Vec4 ZWXW => new Vec4(z, w, x, w);

		/// <summary>A transpose swizzle property, returns (z,w,y,0)</summary>
		public Vec4 ZWY0 => new Vec4(z, w, y, 0);

		/// <summary>A transpose swizzle property, returns (z,w,y,x)</summary>
		public Vec4 ZWYX => new Vec4(z, w, y, x);

		/// <summary>A transpose swizzle property, returns (z,w,y,y)</summary>
		public Vec4 ZWYY => new Vec4(z, w, y, y);

		/// <summary>A transpose swizzle property, returns (z,w,y,z)</summary>
		public Vec4 ZWYZ => new Vec4(z, w, y, z);

		/// <summary>A transpose swizzle property, returns (z,w,y,w)</summary>
		public Vec4 ZWYW => new Vec4(z, w, y, w);

		/// <summary>A transpose swizzle property, returns (z,w,z,0)</summary>
		public Vec4 ZWZ0 => new Vec4(z, w, z, 0);

		/// <summary>A transpose swizzle property, returns (z,w,z,x)</summary>
		public Vec4 ZWZX => new Vec4(z, w, z, x);

		/// <summary>A transpose swizzle property, returns (z,w,z,y)</summary>
		public Vec4 ZWZY => new Vec4(z, w, z, y);

		/// <summary>A transpose swizzle property, returns (z,w,z,z)</summary>
		public Vec4 ZWZZ => new Vec4(z, w, z, z);

		/// <summary>A transpose swizzle property, returns (z,w,z,w)</summary>
		public Vec4 ZWZW => new Vec4(z, w, z, w);

		/// <summary>A transpose swizzle property, returns (z,w,w,0)</summary>
		public Vec4 ZWW0 => new Vec4(z, w, w, 0);

		/// <summary>A transpose swizzle property, returns (z,w,w,x)</summary>
		public Vec4 ZWWX => new Vec4(z, w, w, x);

		/// <summary>A transpose swizzle property, returns (z,w,w,y)</summary>
		public Vec4 ZWWY => new Vec4(z, w, w, y);

		/// <summary>A transpose swizzle property, returns (z,w,w,z)</summary>
		public Vec4 ZWWZ => new Vec4(z, w, w, z);

		/// <summary>A transpose swizzle property, returns (z,w,w,w)</summary>
		public Vec4 ZWWW => new Vec4(z, w, w, w);

		/// <summary>A transpose swizzle property, returns (w,0,0,0)</summary>
		public Vec4 W000 => new Vec4(w, 0, 0, 0);

		/// <summary>A transpose swizzle property, returns (w,0,0,x)</summary>
		public Vec4 W00X => new Vec4(w, 0, 0, x);

		/// <summary>A transpose swizzle property, returns (w,0,0,y)</summary>
		public Vec4 W00Y => new Vec4(w, 0, 0, y);

		/// <summary>A transpose swizzle property, returns (w,0,0,z)</summary>
		public Vec4 W00Z => new Vec4(w, 0, 0, z);

		/// <summary>A transpose swizzle property, returns (w,0,0,w)</summary>
		public Vec4 W00W => new Vec4(w, 0, 0, w);

		/// <summary>A transpose swizzle property, returns (w,0,x,0)</summary>
		public Vec4 W0X0 => new Vec4(w, 0, x, 0);

		/// <summary>A transpose swizzle property, returns (w,0,x,x)</summary>
		public Vec4 W0XX => new Vec4(w, 0, x, x);

		/// <summary>A transpose swizzle property, returns (w,0,x,y)</summary>
		public Vec4 W0XY => new Vec4(w, 0, x, y);

		/// <summary>A transpose swizzle property, returns (w,0,x,z)</summary>
		public Vec4 W0XZ => new Vec4(w, 0, x, z);

		/// <summary>A transpose swizzle property, returns (w,0,x,w)</summary>
		public Vec4 W0XW => new Vec4(w, 0, x, w);

		/// <summary>A transpose swizzle property, returns (w,0,y,0)</summary>
		public Vec4 W0Y0 => new Vec4(w, 0, y, 0);

		/// <summary>A transpose swizzle property, returns (w,0,y,x)</summary>
		public Vec4 W0YX => new Vec4(w, 0, y, x);

		/// <summary>A transpose swizzle property, returns (w,0,y,y)</summary>
		public Vec4 W0YY => new Vec4(w, 0, y, y);

		/// <summary>A transpose swizzle property, returns (w,0,y,z)</summary>
		public Vec4 W0YZ => new Vec4(w, 0, y, z);

		/// <summary>A transpose swizzle property, returns (w,0,y,w)</summary>
		public Vec4 W0YW => new Vec4(w, 0, y, w);

		/// <summary>A transpose swizzle property, returns (w,0,z,0)</summary>
		public Vec4 W0Z0 => new Vec4(w, 0, z, 0);

		/// <summary>A transpose swizzle property, returns (w,0,z,x)</summary>
		public Vec4 W0ZX => new Vec4(w, 0, z, x);

		/// <summary>A transpose swizzle property, returns (w,0,z,y)</summary>
		public Vec4 W0ZY => new Vec4(w, 0, z, y);

		/// <summary>A transpose swizzle property, returns (w,0,z,z)</summary>
		public Vec4 W0ZZ => new Vec4(w, 0, z, z);

		/// <summary>A transpose swizzle property, returns (w,0,z,w)</summary>
		public Vec4 W0ZW => new Vec4(w, 0, z, w);

		/// <summary>A transpose swizzle property, returns (w,0,w,0)</summary>
		public Vec4 W0W0 => new Vec4(w, 0, w, 0);

		/// <summary>A transpose swizzle property, returns (w,0,w,x)</summary>
		public Vec4 W0WX => new Vec4(w, 0, w, x);

		/// <summary>A transpose swizzle property, returns (w,0,w,y)</summary>
		public Vec4 W0WY => new Vec4(w, 0, w, y);

		/// <summary>A transpose swizzle property, returns (w,0,w,z)</summary>
		public Vec4 W0WZ => new Vec4(w, 0, w, z);

		/// <summary>A transpose swizzle property, returns (w,0,w,w)</summary>
		public Vec4 W0WW => new Vec4(w, 0, w, w);

		/// <summary>A transpose swizzle property, returns (w,x,0,0)</summary>
		public Vec4 WX00 => new Vec4(w, x, 0, 0);

		/// <summary>A transpose swizzle property, returns (w,x,0,x)</summary>
		public Vec4 WX0X => new Vec4(w, x, 0, x);

		/// <summary>A transpose swizzle property, returns (w,x,0,y)</summary>
		public Vec4 WX0Y => new Vec4(w, x, 0, y);

		/// <summary>A transpose swizzle property, returns (w,x,0,z)</summary>
		public Vec4 WX0Z => new Vec4(w, x, 0, z);

		/// <summary>A transpose swizzle property, returns (w,x,0,w)</summary>
		public Vec4 WX0W => new Vec4(w, x, 0, w);

		/// <summary>A transpose swizzle property, returns (w,x,x,0)</summary>
		public Vec4 WXX0 => new Vec4(w, x, x, 0);

		/// <summary>A transpose swizzle property, returns (w,x,x,x)</summary>
		public Vec4 WXXX => new Vec4(w, x, x, x);

		/// <summary>A transpose swizzle property, returns (w,x,x,y)</summary>
		public Vec4 WXXY => new Vec4(w, x, x, y);

		/// <summary>A transpose swizzle property, returns (w,x,x,z)</summary>
		public Vec4 WXXZ => new Vec4(w, x, x, z);

		/// <summary>A transpose swizzle property, returns (w,x,x,w)</summary>
		public Vec4 WXXW => new Vec4(w, x, x, w);

		/// <summary>A transpose swizzle property, returns (w,x,y,0)</summary>
		public Vec4 WXY0 => new Vec4(w, x, y, 0);

		/// <summary>A transpose swizzle property, returns (w,x,y,x)</summary>
		public Vec4 WXYX => new Vec4(w, x, y, x);

		/// <summary>A transpose swizzle property, returns (w,x,y,y)</summary>
		public Vec4 WXYY => new Vec4(w, x, y, y);

		/// <summary>A transpose swizzle property, returns (w,x,y,z)</summary>
		public Vec4 WXYZ => new Vec4(w, x, y, z);

		/// <summary>A transpose swizzle property, returns (w,x,y,w)</summary>
		public Vec4 WXYW => new Vec4(w, x, y, w);

		/// <summary>A transpose swizzle property, returns (w,x,z,0)</summary>
		public Vec4 WXZ0 => new Vec4(w, x, z, 0);

		/// <summary>A transpose swizzle property, returns (w,x,z,x)</summary>
		public Vec4 WXZX => new Vec4(w, x, z, x);

		/// <summary>A transpose swizzle property, returns (w,x,z,y)</summary>
		public Vec4 WXZY => new Vec4(w, x, z, y);

		/// <summary>A transpose swizzle property, returns (w,x,z,z)</summary>
		public Vec4 WXZZ => new Vec4(w, x, z, z);

		/// <summary>A transpose swizzle property, returns (w,x,z,w)</summary>
		public Vec4 WXZW => new Vec4(w, x, z, w);

		/// <summary>A transpose swizzle property, returns (w,x,w,0)</summary>
		public Vec4 WXW0 => new Vec4(w, x, w, 0);

		/// <summary>A transpose swizzle property, returns (w,x,w,x)</summary>
		public Vec4 WXWX => new Vec4(w, x, w, x);

		/// <summary>A transpose swizzle property, returns (w,x,w,y)</summary>
		public Vec4 WXWY => new Vec4(w, x, w, y);

		/// <summary>A transpose swizzle property, returns (w,x,w,z)</summary>
		public Vec4 WXWZ => new Vec4(w, x, w, z);

		/// <summary>A transpose swizzle property, returns (w,x,w,w)</summary>
		public Vec4 WXWW => new Vec4(w, x, w, w);

		/// <summary>A transpose swizzle property, returns (w,y,0,0)</summary>
		public Vec4 WY00 => new Vec4(w, y, 0, 0);

		/// <summary>A transpose swizzle property, returns (w,y,0,x)</summary>
		public Vec4 WY0X => new Vec4(w, y, 0, x);

		/// <summary>A transpose swizzle property, returns (w,y,0,y)</summary>
		public Vec4 WY0Y => new Vec4(w, y, 0, y);

		/// <summary>A transpose swizzle property, returns (w,y,0,z)</summary>
		public Vec4 WY0Z => new Vec4(w, y, 0, z);

		/// <summary>A transpose swizzle property, returns (w,y,0,w)</summary>
		public Vec4 WY0W => new Vec4(w, y, 0, w);

		/// <summary>A transpose swizzle property, returns (w,y,x,0)</summary>
		public Vec4 WYX0 => new Vec4(w, y, x, 0);

		/// <summary>A transpose swizzle property, returns (w,y,x,x)</summary>
		public Vec4 WYXX => new Vec4(w, y, x, x);

		/// <summary>A transpose swizzle property, returns (w,y,x,y)</summary>
		public Vec4 WYXY => new Vec4(w, y, x, y);

		/// <summary>A transpose swizzle property, returns (w,y,x,z)</summary>
		public Vec4 WYXZ => new Vec4(w, y, x, z);

		/// <summary>A transpose swizzle property, returns (w,y,x,w)</summary>
		public Vec4 WYXW => new Vec4(w, y, x, w);

		/// <summary>A transpose swizzle property, returns (w,y,y,0)</summary>
		public Vec4 WYY0 => new Vec4(w, y, y, 0);

		/// <summary>A transpose swizzle property, returns (w,y,y,x)</summary>
		public Vec4 WYYX => new Vec4(w, y, y, x);

		/// <summary>A transpose swizzle property, returns (w,y,y,y)</summary>
		public Vec4 WYYY => new Vec4(w, y, y, y);

		/// <summary>A transpose swizzle property, returns (w,y,y,z)</summary>
		public Vec4 WYYZ => new Vec4(w, y, y, z);

		/// <summary>A transpose swizzle property, returns (w,y,y,w)</summary>
		public Vec4 WYYW => new Vec4(w, y, y, w);

		/// <summary>A transpose swizzle property, returns (w,y,z,0)</summary>
		public Vec4 WYZ0 => new Vec4(w, y, z, 0);

		/// <summary>A transpose swizzle property, returns (w,y,z,x)</summary>
		public Vec4 WYZX => new Vec4(w, y, z, x);

		/// <summary>A transpose swizzle property, returns (w,y,z,y)</summary>
		public Vec4 WYZY => new Vec4(w, y, z, y);

		/// <summary>A transpose swizzle property, returns (w,y,z,z)</summary>
		public Vec4 WYZZ => new Vec4(w, y, z, z);

		/// <summary>A transpose swizzle property, returns (w,y,z,w)</summary>
		public Vec4 WYZW => new Vec4(w, y, z, w);

		/// <summary>A transpose swizzle property, returns (w,y,w,0)</summary>
		public Vec4 WYW0 => new Vec4(w, y, w, 0);

		/// <summary>A transpose swizzle property, returns (w,y,w,x)</summary>
		public Vec4 WYWX => new Vec4(w, y, w, x);

		/// <summary>A transpose swizzle property, returns (w,y,w,y)</summary>
		public Vec4 WYWY => new Vec4(w, y, w, y);

		/// <summary>A transpose swizzle property, returns (w,y,w,z)</summary>
		public Vec4 WYWZ => new Vec4(w, y, w, z);

		/// <summary>A transpose swizzle property, returns (w,y,w,w)</summary>
		public Vec4 WYWW => new Vec4(w, y, w, w);

		/// <summary>A transpose swizzle property, returns (w,z,0,0)</summary>
		public Vec4 WZ00 => new Vec4(w, z, 0, 0);

		/// <summary>A transpose swizzle property, returns (w,z,0,x)</summary>
		public Vec4 WZ0X => new Vec4(w, z, 0, x);

		/// <summary>A transpose swizzle property, returns (w,z,0,y)</summary>
		public Vec4 WZ0Y => new Vec4(w, z, 0, y);

		/// <summary>A transpose swizzle property, returns (w,z,0,z)</summary>
		public Vec4 WZ0Z => new Vec4(w, z, 0, z);

		/// <summary>A transpose swizzle property, returns (w,z,0,w)</summary>
		public Vec4 WZ0W => new Vec4(w, z, 0, w);

		/// <summary>A transpose swizzle property, returns (w,z,x,0)</summary>
		public Vec4 WZX0 => new Vec4(w, z, x, 0);

		/// <summary>A transpose swizzle property, returns (w,z,x,x)</summary>
		public Vec4 WZXX => new Vec4(w, z, x, x);

		/// <summary>A transpose swizzle property, returns (w,z,x,y)</summary>
		public Vec4 WZXY => new Vec4(w, z, x, y);

		/// <summary>A transpose swizzle property, returns (w,z,x,z)</summary>
		public Vec4 WZXZ => new Vec4(w, z, x, z);

		/// <summary>A transpose swizzle property, returns (w,z,x,w)</summary>
		public Vec4 WZXW => new Vec4(w, z, x, w);

		/// <summary>A transpose swizzle property, returns (w,z,y,0)</summary>
		public Vec4 WZY0 => new Vec4(w, z, y, 0);

		/// <summary>A transpose swizzle property, returns (w,z,y,x)</summary>
		public Vec4 WZYX => new Vec4(w, z, y, x);

		/// <summary>A transpose swizzle property, returns (w,z,y,y)</summary>
		public Vec4 WZYY => new Vec4(w, z, y, y);

		/// <summary>A transpose swizzle property, returns (w,z,y,z)</summary>
		public Vec4 WZYZ => new Vec4(w, z, y, z);

		/// <summary>A transpose swizzle property, returns (w,z,y,w)</summary>
		public Vec4 WZYW => new Vec4(w, z, y, w);

		/// <summary>A transpose swizzle property, returns (w,z,z,0)</summary>
		public Vec4 WZZ0 => new Vec4(w, z, z, 0);

		/// <summary>A transpose swizzle property, returns (w,z,z,x)</summary>
		public Vec4 WZZX => new Vec4(w, z, z, x);

		/// <summary>A transpose swizzle property, returns (w,z,z,y)</summary>
		public Vec4 WZZY => new Vec4(w, z, z, y);

		/// <summary>A transpose swizzle property, returns (w,z,z,z)</summary>
		public Vec4 WZZZ => new Vec4(w, z, z, z);

		/// <summary>A transpose swizzle property, returns (w,z,z,w)</summary>
		public Vec4 WZZW => new Vec4(w, z, z, w);

		/// <summary>A transpose swizzle property, returns (w,z,w,0)</summary>
		public Vec4 WZW0 => new Vec4(w, z, w, 0);

		/// <summary>A transpose swizzle property, returns (w,z,w,x)</summary>
		public Vec4 WZWX => new Vec4(w, z, w, x);

		/// <summary>A transpose swizzle property, returns (w,z,w,y)</summary>
		public Vec4 WZWY => new Vec4(w, z, w, y);

		/// <summary>A transpose swizzle property, returns (w,z,w,z)</summary>
		public Vec4 WZWZ => new Vec4(w, z, w, z);

		/// <summary>A transpose swizzle property, returns (w,z,w,w)</summary>
		public Vec4 WZWW => new Vec4(w, z, w, w);

		/// <summary>A transpose swizzle property, returns (w,w,0,0)</summary>
		public Vec4 WW00 => new Vec4(w, w, 0, 0);

		/// <summary>A transpose swizzle property, returns (w,w,0,x)</summary>
		public Vec4 WW0X => new Vec4(w, w, 0, x);

		/// <summary>A transpose swizzle property, returns (w,w,0,y)</summary>
		public Vec4 WW0Y => new Vec4(w, w, 0, y);

		/// <summary>A transpose swizzle property, returns (w,w,0,z)</summary>
		public Vec4 WW0Z => new Vec4(w, w, 0, z);

		/// <summary>A transpose swizzle property, returns (w,w,0,w)</summary>
		public Vec4 WW0W => new Vec4(w, w, 0, w);

		/// <summary>A transpose swizzle property, returns (w,w,x,0)</summary>
		public Vec4 WWX0 => new Vec4(w, w, x, 0);

		/// <summary>A transpose swizzle property, returns (w,w,x,x)</summary>
		public Vec4 WWXX => new Vec4(w, w, x, x);

		/// <summary>A transpose swizzle property, returns (w,w,x,y)</summary>
		public Vec4 WWXY => new Vec4(w, w, x, y);

		/// <summary>A transpose swizzle property, returns (w,w,x,z)</summary>
		public Vec4 WWXZ => new Vec4(w, w, x, z);

		/// <summary>A transpose swizzle property, returns (w,w,x,w)</summary>
		public Vec4 WWXW => new Vec4(w, w, x, w);

		/// <summary>A transpose swizzle property, returns (w,w,y,0)</summary>
		public Vec4 WWY0 => new Vec4(w, w, y, 0);

		/// <summary>A transpose swizzle property, returns (w,w,y,x)</summary>
		public Vec4 WWYX => new Vec4(w, w, y, x);

		/// <summary>A transpose swizzle property, returns (w,w,y,y)</summary>
		public Vec4 WWYY => new Vec4(w, w, y, y);

		/// <summary>A transpose swizzle property, returns (w,w,y,z)</summary>
		public Vec4 WWYZ => new Vec4(w, w, y, z);

		/// <summary>A transpose swizzle property, returns (w,w,y,w)</summary>
		public Vec4 WWYW => new Vec4(w, w, y, w);

		/// <summary>A transpose swizzle property, returns (w,w,z,0)</summary>
		public Vec4 WWZ0 => new Vec4(w, w, z, 0);

		/// <summary>A transpose swizzle property, returns (w,w,z,x)</summary>
		public Vec4 WWZX => new Vec4(w, w, z, x);

		/// <summary>A transpose swizzle property, returns (w,w,z,y)</summary>
		public Vec4 WWZY => new Vec4(w, w, z, y);

		/// <summary>A transpose swizzle property, returns (w,w,z,z)</summary>
		public Vec4 WWZZ => new Vec4(w, w, z, z);

		/// <summary>A transpose swizzle property, returns (w,w,z,w)</summary>
		public Vec4 WWZW => new Vec4(w, w, z, w);

		/// <summary>A transpose swizzle property, returns (w,w,w,0)</summary>
		public Vec4 WWW0 => new Vec4(w, w, w, 0);

		/// <summary>A transpose swizzle property, returns (w,w,w,x)</summary>
		public Vec4 WWWX => new Vec4(w, w, w, x);

		/// <summary>A transpose swizzle property, returns (w,w,w,y)</summary>
		public Vec4 WWWY => new Vec4(w, w, w, y);

		/// <summary>A transpose swizzle property, returns (w,w,w,z)</summary>
		public Vec4 WWWZ => new Vec4(w, w, w, z);

		/// <summary>A transpose swizzle property, returns (w,w,w,w)</summary>
		public Vec4 WWWW => new Vec4(w, w, w, w);

	}
}
