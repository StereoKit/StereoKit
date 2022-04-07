using System.Diagnostics.Contracts;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary> Bounds is an axis aligned bounding box type that can be used
	/// for storing the sizes of objects, calculating containment,
	/// intersections, and more!
	/// 
	/// While the constructor uses a center+dimensions for creating a bounds,
	/// don't forget the static From* methods that allow you to define a Bounds
	/// from different types of data!</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct Bounds
	{
		/// <summary>The exact center of the Bounds!</summary>
		public Vec3 center;
		/// <summary>The total size of the box, from one end to the other. This
		/// is the width, height, and depth of the Bounds.</summary>
		public Vec3 dimensions;

		/// <summary>Creates a bounding box object!</summary>
		/// <param name="center">The exact center of the box.</param>
		/// <param name="totalDimensions">The total size of the box, from one
		/// end to the other. This is the width, height, and depth of the
		/// Bounds.</param>
		public Bounds(Vec3 center, Vec3 totalDimensions)
		{
			this.center = center;
			dimensions = totalDimensions;
		}

		/// <summary>Creates a bounding box object centered around zero!
		/// </summary>
		/// <param name="totalDimensions">The total size of the box, from one
		/// end to the other. This is the width, height, and depth of the
		/// Bounds.</param>
		public Bounds(Vec3 totalDimensions)
		{
			this.center = Vec3.Zero;
			dimensions = totalDimensions;
		}

		/// <summary>Create a bounding box from a corner, plus box dimensions.
		/// </summary>
		/// <param name="bottomLeftBack">The -X,-Y,-Z corner of the box.</param>
		/// <param name="dimensions">The total dimensions of the box.</param>
		/// <returns>A Bounds object that extends from bottomLeftBack to
		/// bottomLeftBack+dimensions.</returns>
		public static Bounds FromCorner(Vec3 bottomLeftBack, Vec3 dimensions)
			=> new Bounds(bottomLeftBack + dimensions/2, dimensions);

		/// <summary>Create a bounding box between two corner points.</summary>
		/// <param name="bottomLeftBack">The -X,-Y,-Z corner of the box.</param>
		/// <param name="topRightFront">The +X,+Y,+Z corner of the box.</param>
		/// <returns>A Bounds object that extends from bottomLeftBack to
		/// topRightFront.</returns>
		public static Bounds FromCorners(Vec3 bottomLeftBack, Vec3 topRightFront)
			=> new Bounds(bottomLeftBack/2 + topRightFront/2, topRightFront-bottomLeftBack);


		/// <summary>Calculate the intersection between a Ray, and these
		/// bounds. Returns false if no intersection occurred, and 'at' will
		/// contain the nearest intersection point to the start of the ray if
		/// an intersection is found!</summary>
		/// <param name="ray">Any Ray in the same coordinate space as the
		/// Bounds.</param>
		/// <param name="at">If the return is true, this point will be the
		/// closest intersection point to the origin of the Ray.</param>
		/// <returns>True if an intersection occurred, false if not.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool Intersect(Ray ray, out Vec3 at)
			=> NativeAPI.bounds_ray_intersect(this, ray, out at);

		/// <summary>Does the Bounds contain the given point? This includes
		/// points that are on the surface of the Bounds.</summary>
		/// <param name="pt">A point in the same coordinate space as the
		/// Bounds.</param>
		/// <returns>True if the point is on, or in the Bounds.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool Contains(Vec3 pt)
			=> NativeAPI.bounds_point_contains(this, pt);

		/// <summary>Does the Bounds contain or intersects with the given line?
		/// </summary>
		/// <param name="linePt1">Start of the line</param>
		/// <param name="linePt2">End of the line</param>
		/// <returns>True if the line is in, or intersects with the bounds.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool Contains(Vec3 linePt1, Vec3 linePt2)
			=> NativeAPI.bounds_line_contains(this, linePt1, linePt2);

		/// <summary>Does the bounds contain or intersect with the given
		/// capsule?</summary>
		/// <param name="linePt1">Start of the capsule.</param>
		/// <param name="linePt2">End of the capsule</param>
		/// <param name="radius">Radius of the capsule.</param>
		/// <returns>True if the capsule is in, or intersects with the 
		/// bounds.</returns>
		public bool Contains(Vec3 linePt1, Vec3 linePt2, float radius)
			=> NativeAPI.bounds_capsule_contains(this, linePt1, linePt2, radius);

		/// <summary>Scale this bounds. It will scale the center as well as	the dimensions!
		///  Modifies this bounds object.</summary>
		/// <remarks>See <seealso cref="Scaled(float)"/> or <seealso cref="Scaled(Vec3)"/>
		/// for a non-mutating alternative</remarks>
		/// <param name="scale">Scale to apply</param>
		public void Scale(float scale)
		{
			center *= scale;
			dimensions *= scale;
		}

		/// <inheritdoc cref="Scale(float)"/>
		public void Scale(Vec3 scale)
		{
			center *= scale;
			dimensions *= scale;
		}

		/// <summary>Scale the bounds.  
		/// It will scale the center as well as	the dimensions!
		/// Returns a new Bounds.</summary>
		/// <remarks>This is equivalent to using the * operator: bounds * scale</remarks>
		/// <param name="scale">Scale</param>
		/// <returns>A new scaled bounds</returns>
		[Pure]
		public Bounds Scaled(float scale) => this * scale;

		/// <inheritdoc cref="Scaled(float)"/>
		[Pure]
		public Bounds Scaled(Vec3 scale) => this * scale;

		[Pure]
		public static Bounds operator *(Bounds a, float b) => new Bounds(a.center * b, a.dimensions * b);

		[Pure]
		public static Bounds operator *(Bounds a, Vec3 b) => new Bounds(a.center * b, a.dimensions * b);

		public override string ToString()
			=> string.Format("[center:{0} dimensions:{1}]", center, dimensions);
	}
}
