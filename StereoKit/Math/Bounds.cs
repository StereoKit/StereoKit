// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2024 Nick Klingensmith
// Copyright (c) 2023-2024 Qualcomm Technologies, Inc.

using System;
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

		/// <summary>From the front, this is the Top (Y+), Left (X+), Center
		/// (Z0) of the bounds. Useful when working with UI layout bounds.
		/// </summary>
		public Vec3 TLC => center + dimensions.XY0/2;

		/// <summary>From the front, this is the Top (Y+), Left (X+), Back (Z+)
		/// of the bounds. Useful when working with UI layout bounds.</summary>
		public Vec3 TLB => center + dimensions/2;

		/// <summary>Creates a bounding box object centered around zero!
		/// </summary>
		/// <param name="totalDimensions">The total size of the box, from one
		/// end to the other. This is the width, height, and depth of the
		/// Bounds.</param>
		public Bounds(Vec3 totalDimensions)
			: this(Vec3.Zero, totalDimensions) { }

		/// <summary>Creates a bounding box object centered around zero!
		/// </summary>
		/// <param name="totalDimensionX">Total size on the X axis.</param>
		/// <param name="totalDimensionY">Total size on the Y axis.</param>
		/// <param name="totalDimensionZ">Total size on the Z axis.</param>
		public Bounds(float totalDimensionX, float totalDimensionY, float totalDimensionZ)
			: this(Vec3.Zero, new Vec3(totalDimensionX, totalDimensionY, totalDimensionZ)) { }

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

		/// <summary>Grow the Bounds to encapsulate the provided point. Returns
		/// the result, and does NOT modify the current bounds.</summary>
		/// <param name="pt">The point to encapsulate! This should be in the
		/// same space as the bounds.</param>
		/// <returns>The bounds that also encapsulate the provided point.
		/// </returns>
		public Bounds Grown (Vec3 pt)
			=> NativeAPI.bounds_grow_to_fit_pt(this, pt);
		/// <summary>Grow the Bounds to encapsulate the provided box after it
		/// has been transformed by the provided matrix transform. This will
		/// transform each corner of the box, and expand the bounds to
		/// encapsulate each point!</summary>
		/// <param name="box">The box to encapsulate! The corners of this box
		/// are transformed, and then used to grow the Bounds.</param>
		/// <param name="boxTransform">The Matrix transform for the box. If
		/// this is just an Identity matrix, you can skip providing a Matrix.
		/// </param>
		/// <returns>The bounds that also encapsulate the provided transformed
		/// box.</returns>
		public Bounds Grown (Bounds box, Matrix boxTransform)
			=> NativeAPI.bounds_grow_to_fit_box(this, box, boxTransform);
		/// <summary>Grow the Bounds to encapsulate the provided box.</summary>
		/// <param name="box">The box to encapsulate!</param>
		/// <returns>The bounds that also encapsulate the provided box.
		/// </returns>
		public Bounds Grown (Bounds box)
			=> NativeAPI.bounds_grow_to_fit_box(this, box, IntPtr.Zero);
		/// <summary>This returns a Bounds that encapsulates the transformed
		/// points of the current Bounds's corners. Note that this will likely
		/// introduce a lot of extra empty volume in many cases, as the result
		/// is still always axis aligned.</summary>
		/// <param name="transform">A transform Matrix for the current Bounds's
		/// corners.</param>
		/// <returns>A Bounds that encapsulates the transformed points of the
		/// current Bounds's corners</returns>
		public Bounds Transformed(Matrix transform)
			=> NativeAPI.bounds_transform(this, transform);

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

		/// <summary>This operator will create a new Bounds that has been
		/// properly scaled up by the float. This does affect the center
		/// position of the Bounds.</summary>
		/// <param name="a">The source Bounds.</param>
		/// <param name="b">A scalar multiplier for the Bounds.</param>
		/// <returns>A new Bounds that has been scaled.</returns>
		[Pure]
		public static Bounds operator *(Bounds a, float b) => new Bounds(a.center * b, a.dimensions * b);

		/// <summary>This operator will create a new Bounds that has been
		/// properly scaled up by the Vec3. This does affect the center
		/// position of the Bounds.</summary>
		/// <param name="a">The source Bounds.</param>
		/// <param name="b">A Vec3 multiplier for the Bounds.</param>
		/// <returns>A new Bounds that has been scaled.</returns>
		[Pure]
		public static Bounds operator *(Bounds a, Vec3 b) => new Bounds(a.center * b, a.dimensions * b);

		/// <summary>Creates a text description of the Bounds, in the format of
		/// "[center:X dimensions:X]"</summary>
		/// <returns>A text description of the Bounds, in the format of
		/// "[center:X dimensions:X]"</returns>
		public override string ToString()
			=> string.Format("[center:{0} dimensions:{1}]", center, dimensions);
	}
}
