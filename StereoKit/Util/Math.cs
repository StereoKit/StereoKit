using System.Runtime.InteropServices;

namespace StereoKit
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vec2
    {
        public float x, y;

        public Vec2(float x, float y)
        { 
            this.x = x;
            this.y = y;
        }
        public override string ToString()
        {
            return string.Format("<{0:0.00}, {1:0.00}>", x, y);
        }

        public static readonly Vec2 Zero = new Vec2(0,0);
        public static readonly Vec2 One  = new Vec2(1,1);
        public static Vec2 operator +(Vec2 a, Vec2 b)  { return new Vec2(a.x + b.x, a.y + b.y); }
        public static Vec2 operator -(Vec2 a, Vec2 b)  { return new Vec2(a.x - b.x, a.y - b.y); }
        public static Vec2 operator -(Vec2 a)          { return new Vec2(-a.x, -a.y); }
        public static Vec2 operator *(Vec2 a, float b) { return new Vec2(a.x * b, a.y * b); }
        public static Vec2 operator /(Vec2 a, float b) { return new Vec2(a.x / b, a.y / b); }
    }

    /// <summary>A vector with 3 components: x, y, z. This can represent a point in space,
    /// a directional vector, or any other sort of value with 3 dimensions to it!</summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct Vec3
    {
        public float x, y, z;

        public Vec3(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }
        public override string ToString()
        {
            return string.Format("<{0:0.00}, {1:0.00}, {2:0.00}>", x, y, z);
        }

        /// <summary>Shorthand for a vector where all values are 0! Same as `new Vec3(0,0,0)`.</summary>
        public static readonly Vec3 Zero    = new Vec3(0,0,0);
        /// <summary>Shorthand for a vector where all values are 1! Same as `new Vec3(1,1,1)`.</summary>
        public static readonly Vec3 One     = new Vec3(1,1,1);
        /// <summary>A vector representing the up axis. In StereoKit, this is the same as `new Vec3(0,1,0)`.</summary>
        public static readonly Vec3 Up      = new Vec3(0,1,0);
        /// <summary>StereoKit uses a right-handed coordinate system, which means that forwrd is 
        /// looking down the -Z axis! This value is the same as `new Vec3(0,0,-1)`</summary>
        public static readonly Vec3 Forward = new Vec3(0,0,-1);
        public static Vec3 operator +(Vec3 a, Vec3 b)  { return new Vec3(a.x + b.x, a.y + b.y, a.z + b.z); }
        public static Vec3 operator -(Vec3 a, Vec3 b)  { return new Vec3(a.x - b.x, a.y - b.y, a.z - b.z); }
        public static Vec3 operator -(Vec3 a)          { return new Vec3(-a.x, -a.y, -a.z); }
        public static Vec3 operator *(Vec3 a, float b) { return new Vec3(a.x * b, a.y * b, a.z * b); }
        public static Vec3 operator /(Vec3 a, float b) { return new Vec3(a.x / b, a.y / b, a.z / b); }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Vec4
    {
        public float x, y, z, w;

        public Vec4(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }
        public override string ToString()
        {
            return string.Format("<{0:0.00}, {1:0.00}, {2:0.00}, {3:0.00}>", x, y, z, w);
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Matrix
    {
        public Vec4 row1;
        public Vec4 row2;
        public Vec4 row3;
        public Vec4 row4;

        public Matrix Inverse() 
        {
            Matrix result; 
            NativeAPI.matrix_inverse(ref this, out result); 
            return result;
        }
        public Vec3 TransformPoint    (Vec3 point)     => NativeAPI.matrix_mul_point    (ref this, ref point);
        public Vec3 TransformDirection(Vec3 direction) => NativeAPI.matrix_mul_direction(ref this, ref direction);

        public static Matrix operator *(Matrix a, Matrix b) { 
            Matrix result;
            NativeAPI.matrix_mul(ref a, ref b, out result);
            return result;
        }
        public static Vec3   operator *(Matrix a, Vec3 b) => NativeAPI.matrix_mul_point(ref a, ref b);

        public static Matrix TRS(Vec3 position, Quat rotation, Vec3 scale) => NativeAPI.matrix_trs(ref position, ref rotation, ref scale);
        public static Matrix Identity { get{
            return new Matrix { 
                row1 = new Vec4(1,0,0,0),
                row2 = new Vec4(0,1,0,0),
                row3 = new Vec4(0,0,1,0),
                row4 = new Vec4(0,0,0,1),
            };
        } }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Ray
    {
        public Vec3 position;
        public Vec3 direction;

        public Ray(Vec3 position, Vec3 direction)
        {
            this.position  = position;
            this.direction = direction;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Pose
    {
        public Vec3 position;
        public Quat orientation;

        public Pose(Vec3 position, Quat orientation)
        {
            this.position    = position;
            this.orientation = orientation;
        }
        
        public Matrix ToMatrix(Vec3 scale)
        {
            return NativeAPI.pose_matrix(this, scale);
        }
        public Matrix ToMatrix()
        {
            return NativeAPI.pose_matrix(this, Vec3.One);
        }
    };
}