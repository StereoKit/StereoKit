using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace StereoKit
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Bounds
    {
        public Vec3 center;
        public Vec3 dimensions;

        public Bounds(Vec3 center, Vec3 totalDimensions)
        {
            this.center = center;
            dimensions = totalDimensions;
        }
        public static Bounds FromCorner(Vec3 bottomLeftBack, Vec3 dimensions)
            => new Bounds(bottomLeftBack + dimensions/2, dimensions);
        public static Bounds FromCorners(Vec3 bottomLeftBack, Vec3 topRightFront)
            => new Bounds(bottomLeftBack/2 + topRightFront/2, topRightFront-bottomLeftBack);
        

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public bool Intersect(Ray ray, out Vec3 at)
            => NativeAPI.bounds_ray_intersect(this, ray, out at);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public bool Contains(Vec3 pt)
            => NativeAPI.bounds_point_contains(this, pt);
    }
}
