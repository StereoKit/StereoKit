using System.Runtime.InteropServices;

namespace StereoKit
{
    /// <summary>A vector with 4 components: x, y, z, and w. Can be useful for things like
    /// shaders, where the registers are aligned to 4 float vectors.</summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct Vec4
    {
        /// <summary>Vector components.</summary>
        public float x, y, z, w;

        public Vec2 XY { get => new Vec2(x, y); set { x = value.x; y = value.y; } }
        public Vec2 YZ { get => new Vec2(y, z); set { y = value.x; z = value.y; } }
        public Vec2 XZ { get => new Vec2(x, z); set { x = value.x; z = value.y; } }
        public Vec2 ZW { get => new Vec2(z, w); set { z = value.x; w = value.y; } }

        /// <summary>A basic constructor, just copies the values in!</summary>
        /// <param name="x">X component of the vector.</param>
        /// <param name="y">Y component of the vector.</param>
        /// <param name="z">Z component of the vector.</param>
        /// <param name="w">W component of the vector.</param>
        public Vec4(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }
        public override string ToString()
        {
            return string.Format("[{0:0.00}, {1:0.00}, {2:0.00}, {3:0.00}]", x, y, z, w);
        }
    }
}
