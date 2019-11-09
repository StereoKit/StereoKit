using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace StereoKit
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Ray
    {
        public Vec3 position;
        public Vec3 direction;

        public Ray(Vec3 position, Vec3 direction)
        {
            this.position = position;
            this.direction = direction;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public bool Intersect(Plane  plane,  out Vec3 at) =>
            NativeAPI.plane_ray_intersect(plane, this, out at);
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public bool Intersect(Sphere sphere, out Vec3 at) =>
            NativeAPI.sphere_ray_intersect(sphere, this, out at);
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public bool Intersect(Bounds bounds, out Vec3 at) =>
            NativeAPI.bounds_ray_intersect(bounds, this, out at);
    }
}
