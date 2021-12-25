
using System;

namespace StereoKit
{
	public partial struct Vec3
	{

		/// <summary>A transpose swizzle property, returns (0,x)</summary>
		public Vec2 OX => new Vec2(0, x);

		/// <summary>A transpose swizzle property, returns (0,y)</summary>
		public Vec2 OY => new Vec2(0, y);

		/// <summary>A transpose swizzle property, returns (0,z)</summary>
		public Vec2 OZ => new Vec2(0, z);

		/// <summary>A transpose swizzle property, returns (x,0)</summary>
		public Vec2 X0 => new Vec2(x, 0);

		/// <summary>A transpose swizzle property, returns (x,x)</summary>
		public Vec2 XX => new Vec2(x, x);

		/// <summary>A transpose swizzle property, returns (x,y)</summary>
		public Vec2 XY => new Vec2(x, y);

		/// <summary>A transpose swizzle property, returns (x,z)</summary>
		public Vec2 XZ => new Vec2(x, z);

		/// <summary>A transpose swizzle property, returns (y,0)</summary>
		public Vec2 Y0 => new Vec2(y, 0);

		/// <summary>A transpose swizzle property, returns (y,x)</summary>
		public Vec2 YX => new Vec2(y, x);

		/// <summary>A transpose swizzle property, returns (y,y)</summary>
		public Vec2 YY => new Vec2(y, y);

		/// <summary>A transpose swizzle property, returns (y,z)</summary>
		public Vec2 YZ => new Vec2(y, z);

		/// <summary>A transpose swizzle property, returns (z,0)</summary>
		public Vec2 Z0 => new Vec2(z, 0);

		/// <summary>A transpose swizzle property, returns (z,x)</summary>
		public Vec2 ZX => new Vec2(z, x);

		/// <summary>A transpose swizzle property, returns (z,y)</summary>
		public Vec2 ZY => new Vec2(z, y);

		/// <summary>A transpose swizzle property, returns (z,z)</summary>
		public Vec2 ZZ => new Vec2(z, z);

		/// <summary>A transpose swizzle property, returns (0,0,x)</summary>
		public Vec3 O0X => new Vec3(0, 0, x);

		/// <summary>A transpose swizzle property, returns (0,0,y)</summary>
		public Vec3 O0Y => new Vec3(0, 0, y);

		/// <summary>A transpose swizzle property, returns (0,0,z)</summary>
		public Vec3 O0Z => new Vec3(0, 0, z);

		/// <summary>A transpose swizzle property, returns (0,x,0)</summary>
		public Vec3 OX0 => new Vec3(0, x, 0);

		/// <summary>A transpose swizzle property, returns (0,x,x)</summary>
		public Vec3 OXX => new Vec3(0, x, x);

		/// <summary>A transpose swizzle property, returns (0,x,y)</summary>
		public Vec3 OXY => new Vec3(0, x, y);

		/// <summary>A transpose swizzle property, returns (0,x,z)</summary>
		public Vec3 OXZ => new Vec3(0, x, z);

		/// <summary>A transpose swizzle property, returns (0,y,0)</summary>
		public Vec3 OY0 => new Vec3(0, y, 0);

		/// <summary>A transpose swizzle property, returns (0,y,x)</summary>
		public Vec3 OYX => new Vec3(0, y, x);

		/// <summary>A transpose swizzle property, returns (0,y,y)</summary>
		public Vec3 OYY => new Vec3(0, y, y);

		/// <summary>A transpose swizzle property, returns (0,y,z)</summary>
		public Vec3 OYZ => new Vec3(0, y, z);

		/// <summary>A transpose swizzle property, returns (0,z,0)</summary>
		public Vec3 OZ0 => new Vec3(0, z, 0);

		/// <summary>A transpose swizzle property, returns (0,z,x)</summary>
		public Vec3 OZX => new Vec3(0, z, x);

		/// <summary>A transpose swizzle property, returns (0,z,y)</summary>
		public Vec3 OZY => new Vec3(0, z, y);

		/// <summary>A transpose swizzle property, returns (0,z,z)</summary>
		public Vec3 OZZ => new Vec3(0, z, z);

		/// <summary>A transpose swizzle property, returns (x,0,0)</summary>
		public Vec3 X00 => new Vec3(x, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,0,x)</summary>
		public Vec3 X0X => new Vec3(x, 0, x);

		/// <summary>A transpose swizzle property, returns (x,0,y)</summary>
		public Vec3 X0Y => new Vec3(x, 0, y);

		/// <summary>A transpose swizzle property, returns (x,0,z)</summary>
		public Vec3 X0Z => new Vec3(x, 0, z);

		/// <summary>A transpose swizzle property, returns (x,x,0)</summary>
		public Vec3 XX0 => new Vec3(x, x, 0);

		/// <summary>A transpose swizzle property, returns (x,x,x)</summary>
		public Vec3 XXX => new Vec3(x, x, x);

		/// <summary>A transpose swizzle property, returns (x,x,y)</summary>
		public Vec3 XXY => new Vec3(x, x, y);

		/// <summary>A transpose swizzle property, returns (x,x,z)</summary>
		public Vec3 XXZ => new Vec3(x, x, z);

		/// <summary>A transpose swizzle property, returns (x,y,0)</summary>
		public Vec3 XY0 => new Vec3(x, y, 0);

		/// <summary>A transpose swizzle property, returns (x,y,x)</summary>
		public Vec3 XYX => new Vec3(x, y, x);

		/// <summary>A transpose swizzle property, returns (x,y,y)</summary>
		public Vec3 XYY => new Vec3(x, y, y);

		/// <summary>A transpose swizzle property, returns (x,y,z)</summary>
		public Vec3 XYZ => new Vec3(x, y, z);

		/// <summary>A transpose swizzle property, returns (x,z,0)</summary>
		public Vec3 XZ0 => new Vec3(x, z, 0);

		/// <summary>A transpose swizzle property, returns (x,z,x)</summary>
		public Vec3 XZX => new Vec3(x, z, x);

		/// <summary>A transpose swizzle property, returns (x,z,y)</summary>
		public Vec3 XZY => new Vec3(x, z, y);

		/// <summary>A transpose swizzle property, returns (x,z,z)</summary>
		public Vec3 XZZ => new Vec3(x, z, z);

		/// <summary>A transpose swizzle property, returns (y,0,0)</summary>
		public Vec3 Y00 => new Vec3(y, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,0,x)</summary>
		public Vec3 Y0X => new Vec3(y, 0, x);

		/// <summary>A transpose swizzle property, returns (y,0,y)</summary>
		public Vec3 Y0Y => new Vec3(y, 0, y);

		/// <summary>A transpose swizzle property, returns (y,0,z)</summary>
		public Vec3 Y0Z => new Vec3(y, 0, z);

		/// <summary>A transpose swizzle property, returns (y,x,0)</summary>
		public Vec3 YX0 => new Vec3(y, x, 0);

		/// <summary>A transpose swizzle property, returns (y,x,x)</summary>
		public Vec3 YXX => new Vec3(y, x, x);

		/// <summary>A transpose swizzle property, returns (y,x,y)</summary>
		public Vec3 YXY => new Vec3(y, x, y);

		/// <summary>A transpose swizzle property, returns (y,x,z)</summary>
		public Vec3 YXZ => new Vec3(y, x, z);

		/// <summary>A transpose swizzle property, returns (y,y,0)</summary>
		public Vec3 YY0 => new Vec3(y, y, 0);

		/// <summary>A transpose swizzle property, returns (y,y,x)</summary>
		public Vec3 YYX => new Vec3(y, y, x);

		/// <summary>A transpose swizzle property, returns (y,y,y)</summary>
		public Vec3 YYY => new Vec3(y, y, y);

		/// <summary>A transpose swizzle property, returns (y,y,z)</summary>
		public Vec3 YYZ => new Vec3(y, y, z);

		/// <summary>A transpose swizzle property, returns (y,z,0)</summary>
		public Vec3 YZ0 => new Vec3(y, z, 0);

		/// <summary>A transpose swizzle property, returns (y,z,x)</summary>
		public Vec3 YZX => new Vec3(y, z, x);

		/// <summary>A transpose swizzle property, returns (y,z,y)</summary>
		public Vec3 YZY => new Vec3(y, z, y);

		/// <summary>A transpose swizzle property, returns (y,z,z)</summary>
		public Vec3 YZZ => new Vec3(y, z, z);

		/// <summary>A transpose swizzle property, returns (z,0,0)</summary>
		public Vec3 Z00 => new Vec3(z, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,0,x)</summary>
		public Vec3 Z0X => new Vec3(z, 0, x);

		/// <summary>A transpose swizzle property, returns (z,0,y)</summary>
		public Vec3 Z0Y => new Vec3(z, 0, y);

		/// <summary>A transpose swizzle property, returns (z,0,z)</summary>
		public Vec3 Z0Z => new Vec3(z, 0, z);

		/// <summary>A transpose swizzle property, returns (z,x,0)</summary>
		public Vec3 ZX0 => new Vec3(z, x, 0);

		/// <summary>A transpose swizzle property, returns (z,x,x)</summary>
		public Vec3 ZXX => new Vec3(z, x, x);

		/// <summary>A transpose swizzle property, returns (z,x,y)</summary>
		public Vec3 ZXY => new Vec3(z, x, y);

		/// <summary>A transpose swizzle property, returns (z,x,z)</summary>
		public Vec3 ZXZ => new Vec3(z, x, z);

		/// <summary>A transpose swizzle property, returns (z,y,0)</summary>
		public Vec3 ZY0 => new Vec3(z, y, 0);

		/// <summary>A transpose swizzle property, returns (z,y,x)</summary>
		public Vec3 ZYX => new Vec3(z, y, x);

		/// <summary>A transpose swizzle property, returns (z,y,y)</summary>
		public Vec3 ZYY => new Vec3(z, y, y);

		/// <summary>A transpose swizzle property, returns (z,y,z)</summary>
		public Vec3 ZYZ => new Vec3(z, y, z);

		/// <summary>A transpose swizzle property, returns (z,z,0)</summary>
		public Vec3 ZZ0 => new Vec3(z, z, 0);

		/// <summary>A transpose swizzle property, returns (z,z,x)</summary>
		public Vec3 ZZX => new Vec3(z, z, x);

		/// <summary>A transpose swizzle property, returns (z,z,y)</summary>
		public Vec3 ZZY => new Vec3(z, z, y);

		/// <summary>A transpose swizzle property, returns (z,z,z)</summary>
		public Vec3 ZZZ => new Vec3(z, z, z);

		/// <summary>A transpose swizzle property, returns (0,0,0,x)</summary>
		public Vec4 O00X => new Vec4(0, 0, 0, x);

		/// <summary>A transpose swizzle property, returns (0,0,0,y)</summary>
		public Vec4 O00Y => new Vec4(0, 0, 0, y);

		/// <summary>A transpose swizzle property, returns (0,0,0,z)</summary>
		public Vec4 O00Z => new Vec4(0, 0, 0, z);

		/// <summary>A transpose swizzle property, returns (0,0,x,0)</summary>
		public Vec4 O0X0 => new Vec4(0, 0, x, 0);

		/// <summary>A transpose swizzle property, returns (0,0,x,x)</summary>
		public Vec4 O0XX => new Vec4(0, 0, x, x);

		/// <summary>A transpose swizzle property, returns (0,0,x,y)</summary>
		public Vec4 O0XY => new Vec4(0, 0, x, y);

		/// <summary>A transpose swizzle property, returns (0,0,x,z)</summary>
		public Vec4 O0XZ => new Vec4(0, 0, x, z);

		/// <summary>A transpose swizzle property, returns (0,0,y,0)</summary>
		public Vec4 O0Y0 => new Vec4(0, 0, y, 0);

		/// <summary>A transpose swizzle property, returns (0,0,y,x)</summary>
		public Vec4 O0YX => new Vec4(0, 0, y, x);

		/// <summary>A transpose swizzle property, returns (0,0,y,y)</summary>
		public Vec4 O0YY => new Vec4(0, 0, y, y);

		/// <summary>A transpose swizzle property, returns (0,0,y,z)</summary>
		public Vec4 O0YZ => new Vec4(0, 0, y, z);

		/// <summary>A transpose swizzle property, returns (0,0,z,0)</summary>
		public Vec4 O0Z0 => new Vec4(0, 0, z, 0);

		/// <summary>A transpose swizzle property, returns (0,0,z,x)</summary>
		public Vec4 O0ZX => new Vec4(0, 0, z, x);

		/// <summary>A transpose swizzle property, returns (0,0,z,y)</summary>
		public Vec4 O0ZY => new Vec4(0, 0, z, y);

		/// <summary>A transpose swizzle property, returns (0,0,z,z)</summary>
		public Vec4 O0ZZ => new Vec4(0, 0, z, z);

		/// <summary>A transpose swizzle property, returns (0,x,0,0)</summary>
		public Vec4 OX00 => new Vec4(0, x, 0, 0);

		/// <summary>A transpose swizzle property, returns (0,x,0,x)</summary>
		public Vec4 OX0X => new Vec4(0, x, 0, x);

		/// <summary>A transpose swizzle property, returns (0,x,0,y)</summary>
		public Vec4 OX0Y => new Vec4(0, x, 0, y);

		/// <summary>A transpose swizzle property, returns (0,x,0,z)</summary>
		public Vec4 OX0Z => new Vec4(0, x, 0, z);

		/// <summary>A transpose swizzle property, returns (0,x,x,0)</summary>
		public Vec4 OXX0 => new Vec4(0, x, x, 0);

		/// <summary>A transpose swizzle property, returns (0,x,x,x)</summary>
		public Vec4 OXXX => new Vec4(0, x, x, x);

		/// <summary>A transpose swizzle property, returns (0,x,x,y)</summary>
		public Vec4 OXXY => new Vec4(0, x, x, y);

		/// <summary>A transpose swizzle property, returns (0,x,x,z)</summary>
		public Vec4 OXXZ => new Vec4(0, x, x, z);

		/// <summary>A transpose swizzle property, returns (0,x,y,0)</summary>
		public Vec4 OXY0 => new Vec4(0, x, y, 0);

		/// <summary>A transpose swizzle property, returns (0,x,y,x)</summary>
		public Vec4 OXYX => new Vec4(0, x, y, x);

		/// <summary>A transpose swizzle property, returns (0,x,y,y)</summary>
		public Vec4 OXYY => new Vec4(0, x, y, y);

		/// <summary>A transpose swizzle property, returns (0,x,y,z)</summary>
		public Vec4 OXYZ => new Vec4(0, x, y, z);

		/// <summary>A transpose swizzle property, returns (0,x,z,0)</summary>
		public Vec4 OXZ0 => new Vec4(0, x, z, 0);

		/// <summary>A transpose swizzle property, returns (0,x,z,x)</summary>
		public Vec4 OXZX => new Vec4(0, x, z, x);

		/// <summary>A transpose swizzle property, returns (0,x,z,y)</summary>
		public Vec4 OXZY => new Vec4(0, x, z, y);

		/// <summary>A transpose swizzle property, returns (0,x,z,z)</summary>
		public Vec4 OXZZ => new Vec4(0, x, z, z);

		/// <summary>A transpose swizzle property, returns (0,y,0,0)</summary>
		public Vec4 OY00 => new Vec4(0, y, 0, 0);

		/// <summary>A transpose swizzle property, returns (0,y,0,x)</summary>
		public Vec4 OY0X => new Vec4(0, y, 0, x);

		/// <summary>A transpose swizzle property, returns (0,y,0,y)</summary>
		public Vec4 OY0Y => new Vec4(0, y, 0, y);

		/// <summary>A transpose swizzle property, returns (0,y,0,z)</summary>
		public Vec4 OY0Z => new Vec4(0, y, 0, z);

		/// <summary>A transpose swizzle property, returns (0,y,x,0)</summary>
		public Vec4 OYX0 => new Vec4(0, y, x, 0);

		/// <summary>A transpose swizzle property, returns (0,y,x,x)</summary>
		public Vec4 OYXX => new Vec4(0, y, x, x);

		/// <summary>A transpose swizzle property, returns (0,y,x,y)</summary>
		public Vec4 OYXY => new Vec4(0, y, x, y);

		/// <summary>A transpose swizzle property, returns (0,y,x,z)</summary>
		public Vec4 OYXZ => new Vec4(0, y, x, z);

		/// <summary>A transpose swizzle property, returns (0,y,y,0)</summary>
		public Vec4 OYY0 => new Vec4(0, y, y, 0);

		/// <summary>A transpose swizzle property, returns (0,y,y,x)</summary>
		public Vec4 OYYX => new Vec4(0, y, y, x);

		/// <summary>A transpose swizzle property, returns (0,y,y,y)</summary>
		public Vec4 OYYY => new Vec4(0, y, y, y);

		/// <summary>A transpose swizzle property, returns (0,y,y,z)</summary>
		public Vec4 OYYZ => new Vec4(0, y, y, z);

		/// <summary>A transpose swizzle property, returns (0,y,z,0)</summary>
		public Vec4 OYZ0 => new Vec4(0, y, z, 0);

		/// <summary>A transpose swizzle property, returns (0,y,z,x)</summary>
		public Vec4 OYZX => new Vec4(0, y, z, x);

		/// <summary>A transpose swizzle property, returns (0,y,z,y)</summary>
		public Vec4 OYZY => new Vec4(0, y, z, y);

		/// <summary>A transpose swizzle property, returns (0,y,z,z)</summary>
		public Vec4 OYZZ => new Vec4(0, y, z, z);

		/// <summary>A transpose swizzle property, returns (0,z,0,0)</summary>
		public Vec4 OZ00 => new Vec4(0, z, 0, 0);

		/// <summary>A transpose swizzle property, returns (0,z,0,x)</summary>
		public Vec4 OZ0X => new Vec4(0, z, 0, x);

		/// <summary>A transpose swizzle property, returns (0,z,0,y)</summary>
		public Vec4 OZ0Y => new Vec4(0, z, 0, y);

		/// <summary>A transpose swizzle property, returns (0,z,0,z)</summary>
		public Vec4 OZ0Z => new Vec4(0, z, 0, z);

		/// <summary>A transpose swizzle property, returns (0,z,x,0)</summary>
		public Vec4 OZX0 => new Vec4(0, z, x, 0);

		/// <summary>A transpose swizzle property, returns (0,z,x,x)</summary>
		public Vec4 OZXX => new Vec4(0, z, x, x);

		/// <summary>A transpose swizzle property, returns (0,z,x,y)</summary>
		public Vec4 OZXY => new Vec4(0, z, x, y);

		/// <summary>A transpose swizzle property, returns (0,z,x,z)</summary>
		public Vec4 OZXZ => new Vec4(0, z, x, z);

		/// <summary>A transpose swizzle property, returns (0,z,y,0)</summary>
		public Vec4 OZY0 => new Vec4(0, z, y, 0);

		/// <summary>A transpose swizzle property, returns (0,z,y,x)</summary>
		public Vec4 OZYX => new Vec4(0, z, y, x);

		/// <summary>A transpose swizzle property, returns (0,z,y,y)</summary>
		public Vec4 OZYY => new Vec4(0, z, y, y);

		/// <summary>A transpose swizzle property, returns (0,z,y,z)</summary>
		public Vec4 OZYZ => new Vec4(0, z, y, z);

		/// <summary>A transpose swizzle property, returns (0,z,z,0)</summary>
		public Vec4 OZZ0 => new Vec4(0, z, z, 0);

		/// <summary>A transpose swizzle property, returns (0,z,z,x)</summary>
		public Vec4 OZZX => new Vec4(0, z, z, x);

		/// <summary>A transpose swizzle property, returns (0,z,z,y)</summary>
		public Vec4 OZZY => new Vec4(0, z, z, y);

		/// <summary>A transpose swizzle property, returns (0,z,z,z)</summary>
		public Vec4 OZZZ => new Vec4(0, z, z, z);

		/// <summary>A transpose swizzle property, returns (x,0,0,0)</summary>
		public Vec4 X000 => new Vec4(x, 0, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,0,0,x)</summary>
		public Vec4 X00X => new Vec4(x, 0, 0, x);

		/// <summary>A transpose swizzle property, returns (x,0,0,y)</summary>
		public Vec4 X00Y => new Vec4(x, 0, 0, y);

		/// <summary>A transpose swizzle property, returns (x,0,0,z)</summary>
		public Vec4 X00Z => new Vec4(x, 0, 0, z);

		/// <summary>A transpose swizzle property, returns (x,0,x,0)</summary>
		public Vec4 X0X0 => new Vec4(x, 0, x, 0);

		/// <summary>A transpose swizzle property, returns (x,0,x,x)</summary>
		public Vec4 X0XX => new Vec4(x, 0, x, x);

		/// <summary>A transpose swizzle property, returns (x,0,x,y)</summary>
		public Vec4 X0XY => new Vec4(x, 0, x, y);

		/// <summary>A transpose swizzle property, returns (x,0,x,z)</summary>
		public Vec4 X0XZ => new Vec4(x, 0, x, z);

		/// <summary>A transpose swizzle property, returns (x,0,y,0)</summary>
		public Vec4 X0Y0 => new Vec4(x, 0, y, 0);

		/// <summary>A transpose swizzle property, returns (x,0,y,x)</summary>
		public Vec4 X0YX => new Vec4(x, 0, y, x);

		/// <summary>A transpose swizzle property, returns (x,0,y,y)</summary>
		public Vec4 X0YY => new Vec4(x, 0, y, y);

		/// <summary>A transpose swizzle property, returns (x,0,y,z)</summary>
		public Vec4 X0YZ => new Vec4(x, 0, y, z);

		/// <summary>A transpose swizzle property, returns (x,0,z,0)</summary>
		public Vec4 X0Z0 => new Vec4(x, 0, z, 0);

		/// <summary>A transpose swizzle property, returns (x,0,z,x)</summary>
		public Vec4 X0ZX => new Vec4(x, 0, z, x);

		/// <summary>A transpose swizzle property, returns (x,0,z,y)</summary>
		public Vec4 X0ZY => new Vec4(x, 0, z, y);

		/// <summary>A transpose swizzle property, returns (x,0,z,z)</summary>
		public Vec4 X0ZZ => new Vec4(x, 0, z, z);

		/// <summary>A transpose swizzle property, returns (x,x,0,0)</summary>
		public Vec4 XX00 => new Vec4(x, x, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,x,0,x)</summary>
		public Vec4 XX0X => new Vec4(x, x, 0, x);

		/// <summary>A transpose swizzle property, returns (x,x,0,y)</summary>
		public Vec4 XX0Y => new Vec4(x, x, 0, y);

		/// <summary>A transpose swizzle property, returns (x,x,0,z)</summary>
		public Vec4 XX0Z => new Vec4(x, x, 0, z);

		/// <summary>A transpose swizzle property, returns (x,x,x,0)</summary>
		public Vec4 XXX0 => new Vec4(x, x, x, 0);

		/// <summary>A transpose swizzle property, returns (x,x,x,x)</summary>
		public Vec4 XXXX => new Vec4(x, x, x, x);

		/// <summary>A transpose swizzle property, returns (x,x,x,y)</summary>
		public Vec4 XXXY => new Vec4(x, x, x, y);

		/// <summary>A transpose swizzle property, returns (x,x,x,z)</summary>
		public Vec4 XXXZ => new Vec4(x, x, x, z);

		/// <summary>A transpose swizzle property, returns (x,x,y,0)</summary>
		public Vec4 XXY0 => new Vec4(x, x, y, 0);

		/// <summary>A transpose swizzle property, returns (x,x,y,x)</summary>
		public Vec4 XXYX => new Vec4(x, x, y, x);

		/// <summary>A transpose swizzle property, returns (x,x,y,y)</summary>
		public Vec4 XXYY => new Vec4(x, x, y, y);

		/// <summary>A transpose swizzle property, returns (x,x,y,z)</summary>
		public Vec4 XXYZ => new Vec4(x, x, y, z);

		/// <summary>A transpose swizzle property, returns (x,x,z,0)</summary>
		public Vec4 XXZ0 => new Vec4(x, x, z, 0);

		/// <summary>A transpose swizzle property, returns (x,x,z,x)</summary>
		public Vec4 XXZX => new Vec4(x, x, z, x);

		/// <summary>A transpose swizzle property, returns (x,x,z,y)</summary>
		public Vec4 XXZY => new Vec4(x, x, z, y);

		/// <summary>A transpose swizzle property, returns (x,x,z,z)</summary>
		public Vec4 XXZZ => new Vec4(x, x, z, z);

		/// <summary>A transpose swizzle property, returns (x,y,0,0)</summary>
		public Vec4 XY00 => new Vec4(x, y, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,y,0,x)</summary>
		public Vec4 XY0X => new Vec4(x, y, 0, x);

		/// <summary>A transpose swizzle property, returns (x,y,0,y)</summary>
		public Vec4 XY0Y => new Vec4(x, y, 0, y);

		/// <summary>A transpose swizzle property, returns (x,y,0,z)</summary>
		public Vec4 XY0Z => new Vec4(x, y, 0, z);

		/// <summary>A transpose swizzle property, returns (x,y,x,0)</summary>
		public Vec4 XYX0 => new Vec4(x, y, x, 0);

		/// <summary>A transpose swizzle property, returns (x,y,x,x)</summary>
		public Vec4 XYXX => new Vec4(x, y, x, x);

		/// <summary>A transpose swizzle property, returns (x,y,x,y)</summary>
		public Vec4 XYXY => new Vec4(x, y, x, y);

		/// <summary>A transpose swizzle property, returns (x,y,x,z)</summary>
		public Vec4 XYXZ => new Vec4(x, y, x, z);

		/// <summary>A transpose swizzle property, returns (x,y,y,0)</summary>
		public Vec4 XYY0 => new Vec4(x, y, y, 0);

		/// <summary>A transpose swizzle property, returns (x,y,y,x)</summary>
		public Vec4 XYYX => new Vec4(x, y, y, x);

		/// <summary>A transpose swizzle property, returns (x,y,y,y)</summary>
		public Vec4 XYYY => new Vec4(x, y, y, y);

		/// <summary>A transpose swizzle property, returns (x,y,y,z)</summary>
		public Vec4 XYYZ => new Vec4(x, y, y, z);

		/// <summary>A transpose swizzle property, returns (x,y,z,0)</summary>
		public Vec4 XYZ0 => new Vec4(x, y, z, 0);

		/// <summary>A transpose swizzle property, returns (x,y,z,x)</summary>
		public Vec4 XYZX => new Vec4(x, y, z, x);

		/// <summary>A transpose swizzle property, returns (x,y,z,y)</summary>
		public Vec4 XYZY => new Vec4(x, y, z, y);

		/// <summary>A transpose swizzle property, returns (x,y,z,z)</summary>
		public Vec4 XYZZ => new Vec4(x, y, z, z);

		/// <summary>A transpose swizzle property, returns (x,z,0,0)</summary>
		public Vec4 XZ00 => new Vec4(x, z, 0, 0);

		/// <summary>A transpose swizzle property, returns (x,z,0,x)</summary>
		public Vec4 XZ0X => new Vec4(x, z, 0, x);

		/// <summary>A transpose swizzle property, returns (x,z,0,y)</summary>
		public Vec4 XZ0Y => new Vec4(x, z, 0, y);

		/// <summary>A transpose swizzle property, returns (x,z,0,z)</summary>
		public Vec4 XZ0Z => new Vec4(x, z, 0, z);

		/// <summary>A transpose swizzle property, returns (x,z,x,0)</summary>
		public Vec4 XZX0 => new Vec4(x, z, x, 0);

		/// <summary>A transpose swizzle property, returns (x,z,x,x)</summary>
		public Vec4 XZXX => new Vec4(x, z, x, x);

		/// <summary>A transpose swizzle property, returns (x,z,x,y)</summary>
		public Vec4 XZXY => new Vec4(x, z, x, y);

		/// <summary>A transpose swizzle property, returns (x,z,x,z)</summary>
		public Vec4 XZXZ => new Vec4(x, z, x, z);

		/// <summary>A transpose swizzle property, returns (x,z,y,0)</summary>
		public Vec4 XZY0 => new Vec4(x, z, y, 0);

		/// <summary>A transpose swizzle property, returns (x,z,y,x)</summary>
		public Vec4 XZYX => new Vec4(x, z, y, x);

		/// <summary>A transpose swizzle property, returns (x,z,y,y)</summary>
		public Vec4 XZYY => new Vec4(x, z, y, y);

		/// <summary>A transpose swizzle property, returns (x,z,y,z)</summary>
		public Vec4 XZYZ => new Vec4(x, z, y, z);

		/// <summary>A transpose swizzle property, returns (x,z,z,0)</summary>
		public Vec4 XZZ0 => new Vec4(x, z, z, 0);

		/// <summary>A transpose swizzle property, returns (x,z,z,x)</summary>
		public Vec4 XZZX => new Vec4(x, z, z, x);

		/// <summary>A transpose swizzle property, returns (x,z,z,y)</summary>
		public Vec4 XZZY => new Vec4(x, z, z, y);

		/// <summary>A transpose swizzle property, returns (x,z,z,z)</summary>
		public Vec4 XZZZ => new Vec4(x, z, z, z);

		/// <summary>A transpose swizzle property, returns (y,0,0,0)</summary>
		public Vec4 Y000 => new Vec4(y, 0, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,0,0,x)</summary>
		public Vec4 Y00X => new Vec4(y, 0, 0, x);

		/// <summary>A transpose swizzle property, returns (y,0,0,y)</summary>
		public Vec4 Y00Y => new Vec4(y, 0, 0, y);

		/// <summary>A transpose swizzle property, returns (y,0,0,z)</summary>
		public Vec4 Y00Z => new Vec4(y, 0, 0, z);

		/// <summary>A transpose swizzle property, returns (y,0,x,0)</summary>
		public Vec4 Y0X0 => new Vec4(y, 0, x, 0);

		/// <summary>A transpose swizzle property, returns (y,0,x,x)</summary>
		public Vec4 Y0XX => new Vec4(y, 0, x, x);

		/// <summary>A transpose swizzle property, returns (y,0,x,y)</summary>
		public Vec4 Y0XY => new Vec4(y, 0, x, y);

		/// <summary>A transpose swizzle property, returns (y,0,x,z)</summary>
		public Vec4 Y0XZ => new Vec4(y, 0, x, z);

		/// <summary>A transpose swizzle property, returns (y,0,y,0)</summary>
		public Vec4 Y0Y0 => new Vec4(y, 0, y, 0);

		/// <summary>A transpose swizzle property, returns (y,0,y,x)</summary>
		public Vec4 Y0YX => new Vec4(y, 0, y, x);

		/// <summary>A transpose swizzle property, returns (y,0,y,y)</summary>
		public Vec4 Y0YY => new Vec4(y, 0, y, y);

		/// <summary>A transpose swizzle property, returns (y,0,y,z)</summary>
		public Vec4 Y0YZ => new Vec4(y, 0, y, z);

		/// <summary>A transpose swizzle property, returns (y,0,z,0)</summary>
		public Vec4 Y0Z0 => new Vec4(y, 0, z, 0);

		/// <summary>A transpose swizzle property, returns (y,0,z,x)</summary>
		public Vec4 Y0ZX => new Vec4(y, 0, z, x);

		/// <summary>A transpose swizzle property, returns (y,0,z,y)</summary>
		public Vec4 Y0ZY => new Vec4(y, 0, z, y);

		/// <summary>A transpose swizzle property, returns (y,0,z,z)</summary>
		public Vec4 Y0ZZ => new Vec4(y, 0, z, z);

		/// <summary>A transpose swizzle property, returns (y,x,0,0)</summary>
		public Vec4 YX00 => new Vec4(y, x, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,x,0,x)</summary>
		public Vec4 YX0X => new Vec4(y, x, 0, x);

		/// <summary>A transpose swizzle property, returns (y,x,0,y)</summary>
		public Vec4 YX0Y => new Vec4(y, x, 0, y);

		/// <summary>A transpose swizzle property, returns (y,x,0,z)</summary>
		public Vec4 YX0Z => new Vec4(y, x, 0, z);

		/// <summary>A transpose swizzle property, returns (y,x,x,0)</summary>
		public Vec4 YXX0 => new Vec4(y, x, x, 0);

		/// <summary>A transpose swizzle property, returns (y,x,x,x)</summary>
		public Vec4 YXXX => new Vec4(y, x, x, x);

		/// <summary>A transpose swizzle property, returns (y,x,x,y)</summary>
		public Vec4 YXXY => new Vec4(y, x, x, y);

		/// <summary>A transpose swizzle property, returns (y,x,x,z)</summary>
		public Vec4 YXXZ => new Vec4(y, x, x, z);

		/// <summary>A transpose swizzle property, returns (y,x,y,0)</summary>
		public Vec4 YXY0 => new Vec4(y, x, y, 0);

		/// <summary>A transpose swizzle property, returns (y,x,y,x)</summary>
		public Vec4 YXYX => new Vec4(y, x, y, x);

		/// <summary>A transpose swizzle property, returns (y,x,y,y)</summary>
		public Vec4 YXYY => new Vec4(y, x, y, y);

		/// <summary>A transpose swizzle property, returns (y,x,y,z)</summary>
		public Vec4 YXYZ => new Vec4(y, x, y, z);

		/// <summary>A transpose swizzle property, returns (y,x,z,0)</summary>
		public Vec4 YXZ0 => new Vec4(y, x, z, 0);

		/// <summary>A transpose swizzle property, returns (y,x,z,x)</summary>
		public Vec4 YXZX => new Vec4(y, x, z, x);

		/// <summary>A transpose swizzle property, returns (y,x,z,y)</summary>
		public Vec4 YXZY => new Vec4(y, x, z, y);

		/// <summary>A transpose swizzle property, returns (y,x,z,z)</summary>
		public Vec4 YXZZ => new Vec4(y, x, z, z);

		/// <summary>A transpose swizzle property, returns (y,y,0,0)</summary>
		public Vec4 YY00 => new Vec4(y, y, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,y,0,x)</summary>
		public Vec4 YY0X => new Vec4(y, y, 0, x);

		/// <summary>A transpose swizzle property, returns (y,y,0,y)</summary>
		public Vec4 YY0Y => new Vec4(y, y, 0, y);

		/// <summary>A transpose swizzle property, returns (y,y,0,z)</summary>
		public Vec4 YY0Z => new Vec4(y, y, 0, z);

		/// <summary>A transpose swizzle property, returns (y,y,x,0)</summary>
		public Vec4 YYX0 => new Vec4(y, y, x, 0);

		/// <summary>A transpose swizzle property, returns (y,y,x,x)</summary>
		public Vec4 YYXX => new Vec4(y, y, x, x);

		/// <summary>A transpose swizzle property, returns (y,y,x,y)</summary>
		public Vec4 YYXY => new Vec4(y, y, x, y);

		/// <summary>A transpose swizzle property, returns (y,y,x,z)</summary>
		public Vec4 YYXZ => new Vec4(y, y, x, z);

		/// <summary>A transpose swizzle property, returns (y,y,y,0)</summary>
		public Vec4 YYY0 => new Vec4(y, y, y, 0);

		/// <summary>A transpose swizzle property, returns (y,y,y,x)</summary>
		public Vec4 YYYX => new Vec4(y, y, y, x);

		/// <summary>A transpose swizzle property, returns (y,y,y,y)</summary>
		public Vec4 YYYY => new Vec4(y, y, y, y);

		/// <summary>A transpose swizzle property, returns (y,y,y,z)</summary>
		public Vec4 YYYZ => new Vec4(y, y, y, z);

		/// <summary>A transpose swizzle property, returns (y,y,z,0)</summary>
		public Vec4 YYZ0 => new Vec4(y, y, z, 0);

		/// <summary>A transpose swizzle property, returns (y,y,z,x)</summary>
		public Vec4 YYZX => new Vec4(y, y, z, x);

		/// <summary>A transpose swizzle property, returns (y,y,z,y)</summary>
		public Vec4 YYZY => new Vec4(y, y, z, y);

		/// <summary>A transpose swizzle property, returns (y,y,z,z)</summary>
		public Vec4 YYZZ => new Vec4(y, y, z, z);

		/// <summary>A transpose swizzle property, returns (y,z,0,0)</summary>
		public Vec4 YZ00 => new Vec4(y, z, 0, 0);

		/// <summary>A transpose swizzle property, returns (y,z,0,x)</summary>
		public Vec4 YZ0X => new Vec4(y, z, 0, x);

		/// <summary>A transpose swizzle property, returns (y,z,0,y)</summary>
		public Vec4 YZ0Y => new Vec4(y, z, 0, y);

		/// <summary>A transpose swizzle property, returns (y,z,0,z)</summary>
		public Vec4 YZ0Z => new Vec4(y, z, 0, z);

		/// <summary>A transpose swizzle property, returns (y,z,x,0)</summary>
		public Vec4 YZX0 => new Vec4(y, z, x, 0);

		/// <summary>A transpose swizzle property, returns (y,z,x,x)</summary>
		public Vec4 YZXX => new Vec4(y, z, x, x);

		/// <summary>A transpose swizzle property, returns (y,z,x,y)</summary>
		public Vec4 YZXY => new Vec4(y, z, x, y);

		/// <summary>A transpose swizzle property, returns (y,z,x,z)</summary>
		public Vec4 YZXZ => new Vec4(y, z, x, z);

		/// <summary>A transpose swizzle property, returns (y,z,y,0)</summary>
		public Vec4 YZY0 => new Vec4(y, z, y, 0);

		/// <summary>A transpose swizzle property, returns (y,z,y,x)</summary>
		public Vec4 YZYX => new Vec4(y, z, y, x);

		/// <summary>A transpose swizzle property, returns (y,z,y,y)</summary>
		public Vec4 YZYY => new Vec4(y, z, y, y);

		/// <summary>A transpose swizzle property, returns (y,z,y,z)</summary>
		public Vec4 YZYZ => new Vec4(y, z, y, z);

		/// <summary>A transpose swizzle property, returns (y,z,z,0)</summary>
		public Vec4 YZZ0 => new Vec4(y, z, z, 0);

		/// <summary>A transpose swizzle property, returns (y,z,z,x)</summary>
		public Vec4 YZZX => new Vec4(y, z, z, x);

		/// <summary>A transpose swizzle property, returns (y,z,z,y)</summary>
		public Vec4 YZZY => new Vec4(y, z, z, y);

		/// <summary>A transpose swizzle property, returns (y,z,z,z)</summary>
		public Vec4 YZZZ => new Vec4(y, z, z, z);

		/// <summary>A transpose swizzle property, returns (z,0,0,0)</summary>
		public Vec4 Z000 => new Vec4(z, 0, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,0,0,x)</summary>
		public Vec4 Z00X => new Vec4(z, 0, 0, x);

		/// <summary>A transpose swizzle property, returns (z,0,0,y)</summary>
		public Vec4 Z00Y => new Vec4(z, 0, 0, y);

		/// <summary>A transpose swizzle property, returns (z,0,0,z)</summary>
		public Vec4 Z00Z => new Vec4(z, 0, 0, z);

		/// <summary>A transpose swizzle property, returns (z,0,x,0)</summary>
		public Vec4 Z0X0 => new Vec4(z, 0, x, 0);

		/// <summary>A transpose swizzle property, returns (z,0,x,x)</summary>
		public Vec4 Z0XX => new Vec4(z, 0, x, x);

		/// <summary>A transpose swizzle property, returns (z,0,x,y)</summary>
		public Vec4 Z0XY => new Vec4(z, 0, x, y);

		/// <summary>A transpose swizzle property, returns (z,0,x,z)</summary>
		public Vec4 Z0XZ => new Vec4(z, 0, x, z);

		/// <summary>A transpose swizzle property, returns (z,0,y,0)</summary>
		public Vec4 Z0Y0 => new Vec4(z, 0, y, 0);

		/// <summary>A transpose swizzle property, returns (z,0,y,x)</summary>
		public Vec4 Z0YX => new Vec4(z, 0, y, x);

		/// <summary>A transpose swizzle property, returns (z,0,y,y)</summary>
		public Vec4 Z0YY => new Vec4(z, 0, y, y);

		/// <summary>A transpose swizzle property, returns (z,0,y,z)</summary>
		public Vec4 Z0YZ => new Vec4(z, 0, y, z);

		/// <summary>A transpose swizzle property, returns (z,0,z,0)</summary>
		public Vec4 Z0Z0 => new Vec4(z, 0, z, 0);

		/// <summary>A transpose swizzle property, returns (z,0,z,x)</summary>
		public Vec4 Z0ZX => new Vec4(z, 0, z, x);

		/// <summary>A transpose swizzle property, returns (z,0,z,y)</summary>
		public Vec4 Z0ZY => new Vec4(z, 0, z, y);

		/// <summary>A transpose swizzle property, returns (z,0,z,z)</summary>
		public Vec4 Z0ZZ => new Vec4(z, 0, z, z);

		/// <summary>A transpose swizzle property, returns (z,x,0,0)</summary>
		public Vec4 ZX00 => new Vec4(z, x, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,x,0,x)</summary>
		public Vec4 ZX0X => new Vec4(z, x, 0, x);

		/// <summary>A transpose swizzle property, returns (z,x,0,y)</summary>
		public Vec4 ZX0Y => new Vec4(z, x, 0, y);

		/// <summary>A transpose swizzle property, returns (z,x,0,z)</summary>
		public Vec4 ZX0Z => new Vec4(z, x, 0, z);

		/// <summary>A transpose swizzle property, returns (z,x,x,0)</summary>
		public Vec4 ZXX0 => new Vec4(z, x, x, 0);

		/// <summary>A transpose swizzle property, returns (z,x,x,x)</summary>
		public Vec4 ZXXX => new Vec4(z, x, x, x);

		/// <summary>A transpose swizzle property, returns (z,x,x,y)</summary>
		public Vec4 ZXXY => new Vec4(z, x, x, y);

		/// <summary>A transpose swizzle property, returns (z,x,x,z)</summary>
		public Vec4 ZXXZ => new Vec4(z, x, x, z);

		/// <summary>A transpose swizzle property, returns (z,x,y,0)</summary>
		public Vec4 ZXY0 => new Vec4(z, x, y, 0);

		/// <summary>A transpose swizzle property, returns (z,x,y,x)</summary>
		public Vec4 ZXYX => new Vec4(z, x, y, x);

		/// <summary>A transpose swizzle property, returns (z,x,y,y)</summary>
		public Vec4 ZXYY => new Vec4(z, x, y, y);

		/// <summary>A transpose swizzle property, returns (z,x,y,z)</summary>
		public Vec4 ZXYZ => new Vec4(z, x, y, z);

		/// <summary>A transpose swizzle property, returns (z,x,z,0)</summary>
		public Vec4 ZXZ0 => new Vec4(z, x, z, 0);

		/// <summary>A transpose swizzle property, returns (z,x,z,x)</summary>
		public Vec4 ZXZX => new Vec4(z, x, z, x);

		/// <summary>A transpose swizzle property, returns (z,x,z,y)</summary>
		public Vec4 ZXZY => new Vec4(z, x, z, y);

		/// <summary>A transpose swizzle property, returns (z,x,z,z)</summary>
		public Vec4 ZXZZ => new Vec4(z, x, z, z);

		/// <summary>A transpose swizzle property, returns (z,y,0,0)</summary>
		public Vec4 ZY00 => new Vec4(z, y, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,y,0,x)</summary>
		public Vec4 ZY0X => new Vec4(z, y, 0, x);

		/// <summary>A transpose swizzle property, returns (z,y,0,y)</summary>
		public Vec4 ZY0Y => new Vec4(z, y, 0, y);

		/// <summary>A transpose swizzle property, returns (z,y,0,z)</summary>
		public Vec4 ZY0Z => new Vec4(z, y, 0, z);

		/// <summary>A transpose swizzle property, returns (z,y,x,0)</summary>
		public Vec4 ZYX0 => new Vec4(z, y, x, 0);

		/// <summary>A transpose swizzle property, returns (z,y,x,x)</summary>
		public Vec4 ZYXX => new Vec4(z, y, x, x);

		/// <summary>A transpose swizzle property, returns (z,y,x,y)</summary>
		public Vec4 ZYXY => new Vec4(z, y, x, y);

		/// <summary>A transpose swizzle property, returns (z,y,x,z)</summary>
		public Vec4 ZYXZ => new Vec4(z, y, x, z);

		/// <summary>A transpose swizzle property, returns (z,y,y,0)</summary>
		public Vec4 ZYY0 => new Vec4(z, y, y, 0);

		/// <summary>A transpose swizzle property, returns (z,y,y,x)</summary>
		public Vec4 ZYYX => new Vec4(z, y, y, x);

		/// <summary>A transpose swizzle property, returns (z,y,y,y)</summary>
		public Vec4 ZYYY => new Vec4(z, y, y, y);

		/// <summary>A transpose swizzle property, returns (z,y,y,z)</summary>
		public Vec4 ZYYZ => new Vec4(z, y, y, z);

		/// <summary>A transpose swizzle property, returns (z,y,z,0)</summary>
		public Vec4 ZYZ0 => new Vec4(z, y, z, 0);

		/// <summary>A transpose swizzle property, returns (z,y,z,x)</summary>
		public Vec4 ZYZX => new Vec4(z, y, z, x);

		/// <summary>A transpose swizzle property, returns (z,y,z,y)</summary>
		public Vec4 ZYZY => new Vec4(z, y, z, y);

		/// <summary>A transpose swizzle property, returns (z,y,z,z)</summary>
		public Vec4 ZYZZ => new Vec4(z, y, z, z);

		/// <summary>A transpose swizzle property, returns (z,z,0,0)</summary>
		public Vec4 ZZ00 => new Vec4(z, z, 0, 0);

		/// <summary>A transpose swizzle property, returns (z,z,0,x)</summary>
		public Vec4 ZZ0X => new Vec4(z, z, 0, x);

		/// <summary>A transpose swizzle property, returns (z,z,0,y)</summary>
		public Vec4 ZZ0Y => new Vec4(z, z, 0, y);

		/// <summary>A transpose swizzle property, returns (z,z,0,z)</summary>
		public Vec4 ZZ0Z => new Vec4(z, z, 0, z);

		/// <summary>A transpose swizzle property, returns (z,z,x,0)</summary>
		public Vec4 ZZX0 => new Vec4(z, z, x, 0);

		/// <summary>A transpose swizzle property, returns (z,z,x,x)</summary>
		public Vec4 ZZXX => new Vec4(z, z, x, x);

		/// <summary>A transpose swizzle property, returns (z,z,x,y)</summary>
		public Vec4 ZZXY => new Vec4(z, z, x, y);

		/// <summary>A transpose swizzle property, returns (z,z,x,z)</summary>
		public Vec4 ZZXZ => new Vec4(z, z, x, z);

		/// <summary>A transpose swizzle property, returns (z,z,y,0)</summary>
		public Vec4 ZZY0 => new Vec4(z, z, y, 0);

		/// <summary>A transpose swizzle property, returns (z,z,y,x)</summary>
		public Vec4 ZZYX => new Vec4(z, z, y, x);

		/// <summary>A transpose swizzle property, returns (z,z,y,y)</summary>
		public Vec4 ZZYY => new Vec4(z, z, y, y);

		/// <summary>A transpose swizzle property, returns (z,z,y,z)</summary>
		public Vec4 ZZYZ => new Vec4(z, z, y, z);

		/// <summary>A transpose swizzle property, returns (z,z,z,0)</summary>
		public Vec4 ZZZ0 => new Vec4(z, z, z, 0);

		/// <summary>A transpose swizzle property, returns (z,z,z,x)</summary>
		public Vec4 ZZZX => new Vec4(z, z, z, x);

		/// <summary>A transpose swizzle property, returns (z,z,z,y)</summary>
		public Vec4 ZZZY => new Vec4(z, z, z, y);

		/// <summary>A transpose swizzle property, returns (z,z,z,z)</summary>
		public Vec4 ZZZZ => new Vec4(z, z, z, z);

	}
}
