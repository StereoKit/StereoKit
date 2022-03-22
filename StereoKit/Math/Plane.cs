using System.Numerics;
using System.Runtime.CompilerServices;

namespace StereoKit
{
	/// <summary> Planes are really useful for collisions, intersections, and
	/// visibility testing!
	///
	/// This plane is stored using the ax + by + cz + d = 0 formula, where 
	/// the normal is a,b,c, and the d is, well, d. </summary>
	public struct Plane
	{
		/// <summary>The internal, wrapped System.Numerics type. This can be
		/// nice to have around so you can pass its fields as 'ref', which
		/// you can't do with properties. You won't often need this, as
		/// implicit conversions to System.Numerics types are also
		/// provided.</summary>
		public System.Numerics.Plane p;
		/// <summary>The direction the plane is facing.</summary>
		public Vec3 normal { get=>p.Normal; set=>p.Normal = value; }
		/// <summary>The distance/travel along the plane's normal from the 
		/// origin to the surface of the plane.</summary>
		public float d { get=>p.D; set=>p.D = value; }

		public static implicit operator Plane(System.Numerics.Plane p) => new Plane(p.Normal, p.D);
		public static implicit operator System.Numerics.Plane(Plane p) => p.p;

		/// <summary>Creates a Plane directly from the ax + by + cz + d = 0
		/// formula!</summary>
		/// <param name="normal">Direction the plane is facing.</param>
		/// <param name="d">Distance along the normal from the origin to the surface of the plane.</param>
		public Plane(Vec3 normal, float d) 
			=> p = new System.Numerics.Plane(normal, d);
		/// <summary>Creates a plane from a normal, and any point on the plane!</summary>
		/// <param name="pointOnPlane">Any point directly on the surface of the plane.</param>
		/// <param name="planeNormal">Direction the plane is facing.</param>
		public Plane(Vec3 pointOnPlane, Vec3 planeNormal)
			=> p = new System.Numerics.Plane(planeNormal, -Vec3.Dot(pointOnPlane, planeNormal));
		/// <summary>Creates a plane from 3 points that are directly on that plane.</summary>
		/// <param name="pointOnPlane1">First point on the plane.</param>
		/// <param name="pointOnPlane2">Second point on the plane.</param>
		/// <param name="pointOnPlane3">Third point on the plane.</param>
		public Plane(Vec3 pointOnPlane1, Vec3 pointOnPlane2, Vec3 pointOnPlane3)
		{
			Vec3 dir1 = pointOnPlane2 - pointOnPlane1;
			Vec3 dir2 = pointOnPlane2 - pointOnPlane3;
			Vec3 normal = Vec3.Cross(dir1, dir2);
			p = new System.Numerics.Plane(normal, -Vec3.Dot(pointOnPlane2, normal));
		}

		/// <summary>Checks the intersection of a ray with this plane!
		/// </summary>
		/// <param name="ray">Ray we're checking with.</param>
		/// <param name="at">An out parameter that will hold the intersection
		/// point. If there's no intersection, this will be (0,0,0).</param>
		/// <returns>True if there's an intersection, false if not. Refer to
		/// the 'at' parameter for intersection information!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool Intersect(Ray ray, out Vec3 at)
			=> NativeAPI.plane_ray_intersect(this, ray, out at);

		/// <summary>Checks the intersection of a line with this plane!
		/// </summary>
		/// <param name="lineStart">Start of the line.</param>
		/// <param name="lineEnd">End of the line.</param>
		/// <param name="at">An out parameter that will hold the intersection
		/// point. If there's no intersection, this will be (0,0,0).</param>
		/// <returns>True if there's an intersection, false if not. Refer to
		/// the 'at' parameter for intersection information!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public bool Intersect(Vec3 lineStart, Vec3 lineEnd, out Vec3 at)
			=> NativeAPI.plane_line_intersect(this, lineStart, lineEnd, out at);

		/// <summary>Finds the closest point on this plane to the given 
		/// point!</summary>
		/// <param name="to">The point you have that's not necessarily on the
		/// plane.</param>
		/// <returns>The point on the plane that's closest to the 'to'
		/// parameter.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vec3 Closest(Vec3 to)
			=> NativeAPI.plane_point_closest(this, to);

		/// <summary>Creates a plane from a normal, and any point on the 
		/// plane!</summary>
		/// <param name="pointOnPlane">Any point directly on the surface of 
		/// the plane.</param>
		/// <param name="planeNormal">Direction the plane is facing.</param>
		/// <returns>A plane that contains pointOnPlane, and faces
		/// planeNormal.</returns>
		public static Plane FromPoint(Vec3 pointOnPlane, Vec3 planeNormal)
			=> new Plane(pointOnPlane, planeNormal);

		// TODO: Define facing direction
		/// <summary>Creates a plane from 3 points that are directly on that
		/// plane.</summary>
		/// <param name="pointOnPlane1">First point on the plane.</param>
		/// <param name="pointOnPlane2">Second point on the plane.</param>
		/// <param name="pointOnPlane3">Third point on the plane.</param>
		/// <returns>A plane that contains all three points.</returns>
		public static Plane FromPoints(Vec3 pointOnPlane1, Vec3 pointOnPlane2, Vec3 pointOnPlane3)
			=> new Plane(pointOnPlane1, pointOnPlane2, pointOnPlane3);
	}
}
