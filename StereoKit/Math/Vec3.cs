using System.Numerics;
using System.Runtime.CompilerServices;

namespace StereoKit
{
	/// <summary>A vector with 3 components: x, y, z. This can represent a 
	/// point in space, a directional vector, or any other sort of value with
	/// 3 dimensions to it!
	/// 
	/// StereoKit uses a right-handed coordinate system, where +x is to the
	/// right, +y is upwards, and -z is forward.</summary>
	public static class Vec3
	{
		/// <summary>Shorthand for a vector where all values are 0! Same as
		/// `new Vec3(0,0,0)`.</summary>
		public static readonly Vector3 Zero = new Vector3(0, 0, 0);
		/// <summary>Shorthand for a vector where all values are 1! Same as
		/// `new Vec3(1,1,1)`.</summary>
		public static readonly Vector3 One = new Vector3(1, 1, 1);
		/// <summary>A vector representing the up axis. In StereoKit, this is
		/// the same as `new Vec3(0,1,0)`.</summary>
		public static readonly Vector3 Up = new Vector3(0, 1, 0);
		/// <summary>StereoKit uses a right-handed coordinate system, which
		/// means that forward is looking down the -Z axis! This value is the
		/// same as `new Vec3(0,0,-1)`</summary>
		public static readonly Vector3 Forward = new Vector3(0, 0, -1);
		/// <summary>When looking forward, this is the direction to the 
		/// right! In StereoKit, this is the same as `new Vec3(1,0,0)`
		/// </summary>
		public static readonly Vector3 Right = new Vector3(1, 0, 0);

		/// <summary>Calculates the distance between two points in space! 
		/// Make sure they're in the same coordinate space! Uses a Sqrt, so 
		/// it's not blazing fast, prefer DistanceSq when possible.</summary>
		/// <param name="a">The first point.</param>
		/// <param name="b">And the second point!</param>
		/// <returns>Distance between the two points.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float Distance(in Vector3 a, in Vector3 b) => (a-b).Length();

		/// <summary>Calculates the distance between two points in space, but
		/// leaves them squared! Make sure they're in the same coordinate
		/// space! This is a fast function :)</summary>
		/// <param name="a">The first point.</param>
		/// <param name="b">And the second point!</param>
		/// <returns>Distance between the two points, but squared!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static float DistanceSq(in Vector3 a, in Vector3 b) => (a-b).LengthSquared();

		/// <summary>Creates a vector that points out at the given 2D angle! 
		/// This creates the vector on the XZ plane, and allows you to
		/// specify a constant y value.</summary>
		/// <param name="angleDeg">Angle in degrees, starting from (1,0) at
		/// 0, and continuing to (0,1) at 90.</param>
		/// <param name="y">A constant value you can assign to the resulting
		/// vector's y component.</param>
		/// <returns>A Vector pointing at the given angle! If y is zero, this
		/// will be a normalized vector (vector with a length of 1).</returns>
		public static Vector3 AngleXZ(float angleDeg, float y = 0)
			=> new Vector3(SKMath.Cos(angleDeg * Units.deg2rad), y, SKMath.Sin(angleDeg * Units.deg2rad));

		/// <summary>Creates a vector that points out at the given 2D angle!
		/// This creates the vector on the XY plane, and allows you to
		/// specify a constant z value.</summary>
		/// <param name="angleDeg">Angle in degrees, starting from (1,0) at
		/// 0, and continuing to (0,1) at 90.</param>
		/// <param name="z">A constant value you can assign to the resulting
		/// vector's z component.</param>
		/// <returns>A vector pointing at the given angle! If z is zero, this
		/// will be a normalized vector (vector with a length of 1).</returns>
		public static Vector3 AngleXY(float angleDeg, float z = 0)
			=> new Vector3(SKMath.Cos(angleDeg * Units.deg2rad), SKMath.Sin(angleDeg * Units.deg2rad), z);

		/// <summary>Creates a normalized vector (vector with a length of 1)
		/// from the current vector. Will not work properly if the vector has
		/// a length of zero.</summary>
		/// <returns>The normalized (length of 1) vector!</returns>
		public static Vector3 Normalized(this Vector3 v) => v / v.Length();

		/// <summary>Exactly the same as Vec3.Cross, but has some naming
		/// memnonics for getting the order right when trying to find a 
		/// perpendicular vector using the cross product. This'll also make 
		/// it more obvious to read if that's what you're actually going for
		/// when crossing vectors!
		/// 
		/// If you consider a forward vector and an up vector, then the
		/// direction to the right is pretty trivial to imagine in relation
		/// to those vectors!</summary>
		/// <param name="forward">What way are you facing?</param>
		/// <param name="up">What direction is up?</param>
		/// <returns>Your direction to the right! Result is -not- a unit
		/// vector, even if both 'forward'
		/// and 'up' are unit vectors.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 PerpendicularRight(in Vector3 forward, in Vector3 up) => Vector3.Cross(forward, up);
	}
}
