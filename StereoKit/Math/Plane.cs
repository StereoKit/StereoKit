using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace StereoKit
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Plane
    {
        public Vec3  normal;
        public float d;

        public Plane(Vec3 normal, float d)
        {
            this.normal = normal;
            this.d      = d;
        }
        public Plane(Vec3 pointOnPlane, Vec3 planeNormal)
        {
            normal = planeNormal;
            d = Vec3.Dot(pointOnPlane, -planeNormal);
        }
        public Plane(Vec3 pointOnPlane1, Vec3 pointOnPlane2, Vec3 pointOnPlane3)
        {
            Vec3 dir1 = pointOnPlane2 - pointOnPlane1;
            Vec3 dir2 = pointOnPlane2 - pointOnPlane3;
            normal = Vec3.Cross(dir1, dir2);
            d      = Vec3.Dot(pointOnPlane2, -normal);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public bool Intersect(Ray ray, out Vec3 at)
            => NativeAPI.plane_ray_intersect(this, ray, out at);
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public bool Intersect(Vec3 lineStart, Vec3 lineEnd, out Vec3 at)
            => NativeAPI.plane_line_intersect(this, lineStart, lineEnd, out at);
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public Vec3 Closest(Vec3 to)
            => NativeAPI.plane_point_closest(this, to);
    };
}
