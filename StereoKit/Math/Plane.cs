using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace StereoKit
{
	public static class SKPlane
	{
		/// <summary>Checks the intersection of a ray with this plane!
		/// </summary>
		/// <param name="ray">Ray we're checking with.</param>
		/// <param name="at">An out parameter that will hold the intersection
		/// point. If there's no intersection, this will be (0,0,0).</param>
		/// <returns>True if there's an intersetion, false if not. Refer to
		/// the 'at' parameter for intersection information!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static bool Intersect(this Plane p, Ray ray, out Vector3 at)
			=> NativeAPI.plane_ray_intersect(p, ray, out at);

		/// <summary>Checks the intersection of a line with this plane!
		/// </summary>
		/// <param name="lineStart">Start of the line.</param>
		/// <param name="lineEnd">End of the line.</param>
		/// <param name="at">An out parameter that will hold the intersection
		/// point. If there's no intersection, this will be (0,0,0).</param>
		/// <returns>True if there's an intersetion, false if not. Refer to
		/// the 'at' parameter for intersection information!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static bool Intersect(this Plane p, Vector3 lineStart, Vector3 lineEnd, out Vector3 at)
			=> NativeAPI.plane_line_intersect(p, lineStart, lineEnd, out at);

		/// <summary>Finds the closest point on this plane to the given 
		/// point!</summary>
		/// <param name="to">The point you have that's not necessarily on the
		/// plane.</param>
		/// <returns>The point on the plane that's closest to the 'to'
		/// parameter.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 Closest(this Plane p, Vector3 to)
			=> NativeAPI.plane_point_closest(p, to);

		/// <summary>Creates a plane from a normal, and any point on the 
		/// plane!</summary>
		/// <param name="pointOnPlane">Any point directly on the surface of 
		/// the plane.</param>
		/// <param name="planeNormal">Direction the plane is facing.</param>
		public static Plane FromPoint(Vector3 pointOnPlane, Vector3 planeNormal)
			=> new Plane(planeNormal, -Vector3.Dot(pointOnPlane, planeNormal));

		/// <summary>Creates a plane from 3 points that are directly on that
		/// plane.</summary>
		/// <param name="pointOnPlane1">First point on the plane.</param>
		/// <param name="pointOnPlane2">Second point on the plane.</param>
		/// <param name="pointOnPlane3">Third point on the plane.</param>
		public static Plane FromPoints(Vector3 pointOnPlane1, Vector3 pointOnPlane2, Vector3 pointOnPlane3)
		{
			Vector3 dir1 = pointOnPlane2 - pointOnPlane1;
			Vector3 dir2 = pointOnPlane2 - pointOnPlane3;
			Vector3 norm = Vector3.Cross(dir1, dir2);
			return new Plane(norm, -Vector3.Dot(pointOnPlane2, norm));
		}
	}
}
