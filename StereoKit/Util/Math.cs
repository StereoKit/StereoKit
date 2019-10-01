using System.Runtime.InteropServices;

namespace StereoKit
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vec2
    {
        public float x, y;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Vec3
    {
        public float x, y, z;

        public Vec3(float aX, float aY, float aZ)
        {
            x = aX;
            y = aY;
            z = aZ;
        }
        public override string ToString()
        {
            return string.Format("<{0:0.00}, {1:0.00}, {2:0.00}>", x, y, z);
        }

        public static readonly Vec3 Zero = new Vec3(0,0,0);
        public static readonly Vec3 One  = new Vec3(1,1,1);
        public static Vec3 operator +(Vec3 a, Vec3 b) { return new Vec3(a.x + b.x, a.y + b.y, a.z + b.z); }
        public static Vec3 operator -(Vec3 a, Vec3 b) { return new Vec3(a.x - b.x, a.y - b.y, a.z - b.z); }
        public static Vec3 operator -(Vec3 a)         { return new Vec3(-a.x, -a.y, -a.z); }
        public static Vec3 operator *(Vec3 a, float b) { return new Vec3(a.x * b, a.y * b, a.z * b); }
        public static Vec3 operator /(Vec3 a, float b) { return new Vec3(a.x / b, a.y / b, a.z / b); }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Vec4
    {
        public float x, y, z, w;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Quat
    {
        public float i, j, k, a;

        public Quat(float i, float j, float k, float a)
        {
            this.i = i;
            this.j = j;
            this.k = k;
            this.a = a;
        }

        public static readonly Quat Identity = new Quat(0, 0, 0, 1);
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Matrix
    {
        public Vec4 row1;
        public Vec4 row2;
        public Vec4 row3;
        public Vec4 row4;
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
    };
}