using System;
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
		public Vec3 position;
		/// <summary>The direction the ray is facing, typically does not 
		/// require being a unit vector, or normalized direction.</summary>
		public Vec3 direction;

		/// <summary>Basic initialization constructor! Just copies the 
		/// parameters into the fields.</summary>
		/// <param name="position">The position or origin point of the Ray.
		/// </param>
		/// <param name="direction">The direction the ray is facing, 
		/// typically does not require being a unit vector, or normalized 
		/// direction.</param>
		public Ray(Vec3 position, Vec3 direction)
		{
			this.position  = position;
			this.direction = direction;
		}

		/// <summary>Checks the intersection of this ray with a plane!
		/// </summary>
		/// <param name="plane">Any plane you want to intersect with.</param>
		/// <param name="at">An out parameter that will hold the intersection 
		/// point. If there's no intersection, this will be (0,0,0).</param>
		/// <returns>True if there's an intersection, false if not. Refer to
		/// the 'at' parameter for intersection information!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool Intersect(Plane plane, out Vec3 at)
			=> NativeAPI.plane_ray_intersect(plane, this, out at);

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
		public bool Intersect(Sphere sphere, out Vec3 at)
			=> NativeAPI.sphere_ray_intersect(sphere, this, out at);

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
		public bool Intersect(Bounds bounds, out Vec3 at)
			=> NativeAPI.bounds_ray_intersect(bounds, this, out at);

		/// <summary>Checks the intersection point of this ray and a Mesh 
		/// with collision data stored on the CPU. A mesh without collision
		/// data will always return false. Ray must be in model space, 
		/// intersection point will be in model space too. You can use the
		/// inverse of the mesh's world transform matrix to bring the ray
		/// into model space, see the example in the docs!</summary>
		/// <param name="mesh">A mesh containing collision data on the CPU.
		/// You can check this with Mesh.KeepData.</param>
		/// <param name="modelSpaceAt">The intersection point and surface
		/// direction of the ray and the mesh, if an intersection occurs.
		/// This is in model space, and must be transformed back into world
		/// space later. Direction is not guaranteed to be normalized, 
		/// especially if your own model->world transform contains scale/skew
		/// in it.</param>
		/// <returns>True if an intersection occurs, false otherwise!
		/// </returns>
		public bool Intersect(Mesh mesh, out Ray modelSpaceAt)
			=> NativeAPI.mesh_ray_intersect(mesh._inst, this, out modelSpaceAt,IntPtr.Zero) > 0;

		/// <summary>Checks the intersection point of this ray and a Mesh 
		/// with collision data stored on the CPU. A mesh without collision
		/// data will always return false. Ray must be in model space, 
		/// intersection point will be in model space too. You can use the
		/// inverse of the mesh's world transform matrix to bring the ray
		/// into model space, see the example in the docs!</summary>
		/// <param name="mesh">A mesh containing collision data on the CPU.
		/// You can check this with Mesh.KeepData.</param>
		/// <param name="modelSpaceAt">The intersection point and surface
		/// direction of the ray and the mesh, if an intersection occurs.
		/// This is in model space, and must be transformed back into world
		/// space later. Direction is not guaranteed to be normalized, 
		/// especially if your own model->world transform contains scale/skew
		/// in it.</param>
		/// <param name="outStartInds">The index of the first index of the triangle that was hit</param>
		/// <returns>True if an intersection occurs, false otherwise!
		/// </returns>
		public bool Intersect(Mesh mesh, out Ray modelSpaceAt, out uint outStartInds)
			=> NativeAPI.mesh_ray_intersect(mesh._inst, this, out modelSpaceAt, out outStartInds) > 0;


		// TODO: Remove in v0.4
		[Obsolete("Removing in v0.4, replace with the Ray.Intersect overload with a Ray output.")]
		public bool Intersect(Mesh mesh, out Vec3 modelSpaceAt)
		{
			bool result = NativeAPI.mesh_ray_intersect(mesh._inst, this, out Ray intersection, IntPtr.Zero) > 0;
			modelSpaceAt = intersection.position;
			return result;
		}

		/// <summary>Checks the intersection point of this ray and the Solid
		/// flagged Meshes in the Model's visual nodes. Ray must be in model
		/// space, intersection point will be in model space too. You can use
		/// the inverse of the mesh's world transform matrix to bring the ray
		/// into model space, see the example in the docs!</summary>
		/// <param name="model">Any Model that may or may not contain Solid
		/// flagged nodes, and Meshes with collision data.</param>
		/// <param name="modelSpaceAt">The intersection point and surface
		/// direction of the ray and the mesh, if an intersection occurs.
		/// This is in model space, and must be transformed back into world
		/// space later. Direction is not guaranteed to be normalized, 
		/// especially if your own model->world transform contains scale/skew
		/// in it.</param>
		/// <returns>True if an intersection occurs, false otherwise!
		/// </returns>
		public bool Intersect(Model model, out Ray modelSpaceAt)
			=> NativeAPI.model_ray_intersect(model._inst, this, out modelSpaceAt) > 0;

		/// <summary>Calculates the point on the Ray that's closest to the
		/// given point! This can be in front of, or behind the ray's
		/// starting position.</summary>
		/// <param name="to">Any point in the same coordinate space as the 
		/// Ray.</param>
		/// <returns>The point on the ray that's closest to the given point.
		/// </returns>
		public Vec3 Closest(Vec3 to)
			=> NativeAPI.ray_point_closest(this, to);

		/// <summary>A convenience function that creates a ray from point a, 
		/// towards point b. Resulting direction is not normalized.</summary>
		/// <param name="a">Ray starting point.</param>
		/// <param name="b">Location the ray is pointing towards.</param>
		/// <returns>A ray from point a to point b. Not normalized.</returns>
		public static Ray FromTo(Vec3 a, Vec3 b)
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
		public Vec3 At(float percent) => position + direction*percent;

		/// <summary>Mostly for debug purposes, this is a decent way to log or
		/// inspect the Ray in debug mode. Looks like "[position], [direction]"
		/// </summary>
		/// <returns>A string that looks like "[position], [direction]"
		/// </returns>
		public override string ToString()
			=> string.Format("{0}, {1}", position, direction);
	}
}
