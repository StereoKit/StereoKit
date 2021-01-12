using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>A position and a direction indicating a ray through space!
	/// This is a great tool for intersection testing with geometrical 
	/// shapes.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct Ray
	{
		/// <summary>The position or origin point of the Ray.</summary>
		public Vector3 position;
		/// <summary>The direction the ray is facing, typically does not 
		/// require being a unit vector, or normalized direction.</summary>
		public Vector3 direction;

		/// <summary>Basic initialization constructor! Just copies the 
		/// parameters into the fields.</summary>
		/// <param name="position">The position or origin point of the Ray.
		/// </param>
		/// <param name="direction">The direction the ray is facing, 
		/// typically does not require being a unit vector, or normalized 
		/// direction.</param>
		public Ray(Vector3 position, Vector3 direction)
		{
			this.position = position;
			this.direction = direction;
		}

		/// <summary>Checks the intersection of this ray with a plane!
		/// </summary>
		/// <param name="plane">Any plane you want to intersect with.</param>
		/// <param name="at">An out parameter that will hold the intersection 
		/// point. If there's no intersection, this will be (0,0,0).</param>
		/// <returns>True if there's an intersetion, false if not. Refer to
		/// the 'at' parameter for intersection information!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool Intersect(Plane  plane,  out Vector3 at) =>
			NativeAPI.plane_ray_intersect(plane, this, out at);

		/// <summary>Checks the intersection of this ray with a sphere!
		/// </summary>
		/// <param name="sphere">Any Sphere you want to intersect with.
		/// </param>
		/// <param name="at">An out parameter that will hold the closest 
		/// intersection point to the ray's origin. If there's no 
		/// intersection, this will be (0,0,0).</param>
		/// <returns>True if intersection occurs, false if it doesn't. Refer 
		/// to the 'at' parameter for intersection information!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool Intersect(Sphere sphere, out Vector3 at) =>
			NativeAPI.sphere_ray_intersect(sphere, this, out at);

		/// <summary>Checks the intersection of this ray with a bounding box!
		/// </summary>
		/// <param name="bounds">Any Bounds you want to intersect with.
		/// </param>
		/// <param name="at">If the return is true, this point will be the 
		/// closest intersection point to the origin of the Ray. If there's 
		/// no intersection, this will be (0,0,0).</param>
		/// <returns>True if intersection occurs, false if it doesn't. Refer 
		/// to the 'at' parameter for intersection information!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool Intersect(Bounds bounds, out Vector3 at) =>
			NativeAPI.bounds_ray_intersect(bounds, this, out at);

		/// <summary>Checks the intersection point of this ray and a Mesh 
		/// with collision data stored on the CPU. A mesh without collision
		/// data will always return false. Ray must be in model space, 
		/// intersection point will be in model space too. You can use the
		/// inverse of the mesh's world transform matrix to bring the ray
		/// into model space, see the example in the docs!</summary>
		/// <param name="mesh">A mesh containing collision data on the CPU.
		/// You can check this with Mesh.KeepData.</param>
		/// <param name="modelSpaceAt">The intersection point of the ray and
		/// the mesh, if an intersection occurs. This is in model space, and
		/// must be transformed back into world space later.</param>
		/// <returns>True if an intersection occurs, false otherwise!
		/// </returns>
		public bool Intersect(Mesh mesh, out Vector3 modelSpaceAt) =>
			NativeAPI.mesh_ray_intersect(mesh._inst, this, out modelSpaceAt);

		/// <summary>A convenience function that creates a ray from point a, 
		/// towards point b. Resulting direction is not normalized.</summary>
		/// <param name="a">Ray starting point.</param>
		/// <param name="b">Location the ray is pointing towards.</param>
		/// <returns>A ray from point a to point b. Not normalized.</returns>
		public static Ray FromTo(Vector3 a, Vector3 b)
			=> new Ray(a, b-a);

		/// <summary>Gets a point along the ray! This is basically just
		/// position + direction*percent. If Ray.direction is normalized, 
		/// then percent is functionally distance, and can be used to find
		/// the point a certain distance out along the ray.</summary>
		/// <param name="percent">How far along the ray should we get the 
		/// point at? This is in multiples of Ray.direction's magnitude. If
		/// Ray.direction is normalized, this is functionally the distance.
		/// </param>
		/// <returns>The point at position + direction*percent.</returns>
		public Vector3 At(float percent) => position + direction*percent;
	}
}
