using System.Numerics;
using System.Runtime.CompilerServices;

namespace StereoKit
{
	/// <summary>Quaternions are efficient and robust mathematical objects 
	/// for representing rotations! Understanding the details of how a 
	/// quaterion works is not generally necessary for using them 
	/// effectively, so don't worry too much if they seem weird to you.
	/// They're weird to me too.
	/// 
	/// If you're interested in learning the details though, 3Blue1Brown and
	/// Ben Eater have an [excellent interactive lesson](https://eater.net/quaternions)
	/// about them!</summary>
	public static class Quat
	{
		/// <summary>This is the 'multiply by one!' of the quaternion
		/// rotation world. It's basically a default, no rotation quaternion.
		/// </summary>
		public static readonly Quaternion Identity = Quaternion.Identity;

		/// <summary>Creates a rotation from a resting direction, to a
		/// direction indicated by the direction of the two vectors provided
		/// to the function! This is a great function for camera style
		/// rotation, when you know where a camera is, and where you want to
		/// look at. This prevents roll on the Z axis, Up is always (0,1,0)
		/// </summary>
		/// <param name="from">Where the object is.</param>
		/// <param name="to">Where the object should be looking!</param>
		/// <param name="up">Which direction is up from this perspective?
		/// </param>
		/// <returns>A rotation from resting, to the look direction of the 
		/// parameters.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quaternion LookAt(Vector3 from, Vector3 to, Vector3 up) => NativeAPI.quat_lookat_up(from, to, up);

		/// <summary>Creates a rotation from a resting direction, to a
		/// direction indicated by the direction of the two vectors provided
		/// to the function! This is a greatfunction for camera style
		/// rotation, when you know where a camera is, and where you want to
		/// look at. This prevents roll on the Z axis, Up is always (0,1,0).
		/// </summary>
		/// <param name="from">Where the object is.</param>
		/// <param name="to">Where the object should be looking!</param>
		/// <returns>A rotation from resting, to the look direction of the
		/// parameters.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quaternion LookAt(Vector3 from, Vector3 to) => NativeAPI.quat_lookat_up(from, to, Vec3.Up);

		/// <summary>Creates a rotation from a resting direction, to the
		/// given direction! This prevents roll on the Z axis, Up is always
		/// (0,1,0)</summary>
		/// <param name="direction">Direction the rotation should be looking.
		/// Doesn't need to be normalized.</param>
		/// <returns>A rotation from resting, to the given direction.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quaternion LookDir(Vector3 direction) => NativeAPI.quat_lookat(Vector3.Zero, direction);

		/// <summary>Creates a rotation from a resting direction, to the 
		/// given direction! This prevents roll on the Z axis, Up is always 
		/// (0,1,0)</summary>
		/// <param name="x">X component of the direction the rotation should
		/// be looking. Doesn't need to be normalized.</param>
		/// <param name="y">Y component of the direction the rotation should
		/// be looking. Doesn't need to be normalized.</param>
		/// <param name="z">Z component of the direction the rotation should
		/// be looking. Doesn't need to be normalized.</param>
		/// <returns>A rotation from resting, to the given direction.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quaternion LookDir(float x, float y, float z) => NativeAPI.quat_lookat(Vector3.Zero, new Vector3(x, y, z));

		/// <summary>This gives a relative rotation between the first and 
		/// second quaternion rotations.
		/// Remember that order is important here!</summary>
		/// <param name="a">Starting rotation.</param>
		/// <param name="b">Ending rotation.</param>
		/// <returns>A rotation that will take a point from rotation a, to rotation b.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quaternion Difference(Quaternion a, Quaternion b) => NativeAPI.quat_difference(a, b);

		/// <summary>Creates a Roll/Pitch/Yaw rotation (applied in that
		/// order) from the provided angles in degrees!</summary>
		/// <param name="pitchXDeg">Pitch is rotation around the x axis,
		/// measured in degrees.</param>
		/// <param name="yawYDeg">Yaw is rotation around the y axis, measured
		/// in degrees.</param>
		/// <param name="rollZDeg">Roll is rotation around the z axis, 
		/// measured in degrees.</param>
		/// <returns>A quaternion representing the given Roll/Pitch/Yaw 
		/// roatation!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quaternion FromAngles(float pitchXDeg, float yawYDeg, float rollZDeg) => Quaternion.CreateFromYawPitchRoll(yawYDeg * Units.deg2rad, pitchXDeg * Units.deg2rad, rollZDeg * Units.deg2rad);
		/// <summary>Creates a Roll/Pitch/Yaw rotation (applied in that
		/// order) from the provided angles in degrees!</summary>
		/// <param name="pitchYawRollDeg">Pitch, yaw, and roll stored as
		/// X, Y, and Z in this Vector. Angle values are in degrees.</param>
		/// <returns>A quaternion representing the given Roll/Pitch/Yaw 
		/// roatation!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quaternion FromAngles(Vector3 pitchYawRollDeg) => Quaternion.CreateFromYawPitchRoll(pitchYawRollDeg.Y * Units.deg2rad, pitchYawRollDeg.X * Units.deg2rad, pitchYawRollDeg.Z * Units.deg2rad);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 Rotate(this Quaternion q, Vector3 pt) => NativeAPI.quat_mul_vec(q, pt);
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vector3 Rotate(this Vector3 v, Quaternion rotation) => NativeAPI.quat_mul_vec(rotation, v);
	}
}
