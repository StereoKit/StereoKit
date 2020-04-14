using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace StereoKit
{
    /// <summary>A position and a direction indicating a ray through space! This
    /// is a great tool for intersection testing with geometrical shapes.</summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct Ray
    {
        /// <summary>The position or origin point of the Ray.</summary>
        public Vec3 position;
        /// <summary>The direction the ray is facing, typically does not require being
        /// a unit vector, or normalized direction.</summary>
        public Vec3 direction;

        /// <summary>Basic initialization constructor! Just copies the parameters
        /// into the fields.</summary>
        /// <param name="position">The position or origin point of the Ray.</param>
        /// <param name="direction">The direction the ray is facing, typically does not require being
        /// a unit vector, or normalized direction.</param>
        public Ray(Vec3 position, Vec3 direction)
        {
            this.position = position;
            this.direction = direction;
        }

        /// <summary>Checks the intersection of this ray with a plane!</summary>
        /// <param name="plane">Any plane you want to intersect with.</param>
        /// <param name="at">An out parameter that will hold the intersection point. 
        /// If there's no intersection, this will be (0,0,0).</param>
        /// <returns>True if there's an intersetion, false if not. Refer to the 'at'
        /// parameter for intersection information!</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public bool Intersect(Plane  plane,  out Vec3 at) =>
            NativeAPI.plane_ray_intersect(plane, this, out at);

        /// <summary>Checks the intersection of this ray with a sphere!</summary>
        /// <param name="sphere">Any Sphere you want to intersect with.</param>
        /// <param name="at">An out parameter that will hold the closest intersection 
        /// point to the ray's origin. If there's no intersection, this will be (0,0,0).</param>
        /// <returns>True if intersection occurs, false if it doesn't. Refer to the 'at'
        /// parameter for intersection information!</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public bool Intersect(Sphere sphere, out Vec3 at) =>
            NativeAPI.sphere_ray_intersect(sphere, this, out at);

        /// <summary>Checks the intersection of this ray with a bounding box!</summary>
        /// <param name="bounds">Any Bounds you want to intersect with.</param>
        /// <param name="at">If the return is true, this point will be the closest intersection 
        /// point to the origin of the Ray. If there's no intersection, this will be (0,0,0).</param>
        /// <returns>True if intersection occurs, false if it doesn't. Refer to the 'at'
        /// parameter for intersection information!</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public bool Intersect(Bounds bounds, out Vec3 at) =>
            NativeAPI.bounds_ray_intersect(bounds, this, out at);

        public bool Intersect(Mesh mesh, out Vec3 modelSpaceAt) =>
            NativeAPI.mesh_ray_intersect(mesh._inst, this, out modelSpaceAt);

        public static Ray FromTo(Vec3 a, Vec3 b)
            => new Ray(a, b-a);
    }
}
