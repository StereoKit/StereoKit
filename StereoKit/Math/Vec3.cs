
using System.Runtime.InteropServices;

namespace StereoKit
{
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
}
