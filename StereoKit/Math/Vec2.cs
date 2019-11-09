
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
}
