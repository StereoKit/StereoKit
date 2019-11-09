using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace StereoKit
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Sphere
    {
        public Vec3  center;
        public float radius;

        public Sphere(Vec3 center, float diameter)
        {
            this.center = center;
            this.radius = diameter/2;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public bool Intersect(Ray ray, out Vec3 at) =>
            NativeAPI.sphere_ray_intersect(this, ray, out at);

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public bool Contains(Vec3 pt) =>
            NativeAPI.sphere_point_contains(this, pt);
    }
}
