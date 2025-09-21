using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>Represents a sphere in 3D space! Composed of a center point
	/// and a radius, can be used for raycasting, collision, visibility, and
	/// other things!</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct Sphere
	{
		/// <summary>Center of the sphere.</summary>
		public Vec3  center;
		/// <summary>Distance from the center, to the surface of the sphere, in
		/// meters. Half the diameter.</summary>
		public float radius;

		/// <summary>Length of the line passing through the center from one side of 
		/// the sphere to the other, in meters. Twice the radius.</summary>
		public float Diameter { 
			get { return radius * 2; } 
			set { radius = value/2;  }
		}

		/// <summary>Creates a sphere using a center point and a diameter!</summary>
		/// <param name="center">Center of the sphere.</param>
		/// <param name="diameter">Diameter is in meters. Twice the radius, the 
		/// distance from one side of the sphere to the other when drawing a line 
		/// through the center of the sphere.</param>
		public Sphere(Vec3 center, float diameter)
		{
			this.center = center;
			this.radius = diameter/2;
		}

		/// <summary>Intersects a ray with this sphere, and finds if they intersect, 
		/// and if so, where that intersection is! This only finds the closest 
		/// intersection point to the origin of the ray.</summary>
		/// <param name="ray">A ray to intersect with.</param>
		/// <param name="at">An out parameter that will hold the closest intersection 
		/// point to the ray's origin. If there's not intersection, this will be (0,0,0).
		/// </param>
		/// <returns>True if intersection occurs, false if it doesn't. Refer to the 'at'
		/// parameter for intersection information!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool Intersect(Ray ray, out Vec3 at)
		{ unsafe {
			fixed (Vec3* atPtr = &at)
			return NB.Bool(NativeAPI.sphere_ray_intersect(this, ray, atPtr));
		} }


		/// <summary>A fast check to see if the given point is contained in or on 
		/// a sphere!</summary>
		/// <param name="point">A point in the same coordinate space as this sphere.</param>
		/// <returns>True if in or on the sphere, false if outside.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool Contains(Vec3 point)
			=> NB.Bool(NativeAPI.sphere_point_contains(this, point));
	}
}
