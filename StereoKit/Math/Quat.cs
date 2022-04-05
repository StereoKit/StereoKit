using System.Numerics;
using System.Runtime.CompilerServices;

namespace StereoKit
{
	/// <summary>Quaternions are efficient and robust mathematical objects for
	/// representing rotations! Understanding the details of how a quaternion
	/// works is not generally necessary for using them effectively, so don't
	/// worry too much if they seem weird to you. They're weird to me too.
	/// 
	/// If you're interested in learning the details though, 3Blue1Brown and
	/// Ben Eater have an [excellent interactive lesson](https://eater.net/quaternions)
	/// about them!</summary>
	public struct Quat
	{
		/// <summary>The internal, wrapped System.Numerics type. This can be
		/// nice to have around so you can pass its fields as 'ref', which you
		/// can't do with properties. You won't often need this, as implicit
		/// conversions to System.Numerics types are also provided.</summary>
		public Quaternion q;
		/// <summary>X component. Sometimes known as I.</summary>
		public float x { get => q.X; set => q.X = value; }
		/// <summary>Y component. Sometimes known as J.</summary>
		public float y { get => q.Y; set => q.Y = value; }
		/// <summary>Z component. Sometimes known as K.</summary>
		public float z { get => q.Z; set => q.Z = value; }
		/// <summary>W component.</summary>
		public float w { get => q.W; set => q.W = value; }

		/// <summary>Sometimes you want to do weird stuff with your
		/// Quaternions. I won't judge. This just turns the Quat into a Vec4,
		/// makes some types of math easier!</summary>
		public Vec4 Vec4 => new Vec4(x, y, z, w);

		/// <summary>You may want to use static creation methods, like
		/// Quat.LookAt, or Quat.Identity instead of this one! Unless you
		/// know what you're doing.</summary>
		/// <param name="x">X component of the Quat.</param>
		/// <param name="y">Y component of the Quat.</param>
		/// <param name="z">Z component of the Quat.</param>
		/// <param name="w">W component of the Quat.</param>
		public Quat(float x, float y, float z, float w) 
			=> q = new Quaternion(x,y,z,w);

		/// <summary>Allows implicit conversion from System.Numerics.Quaternion
		/// to StereoKit.Quat.</summary>
		/// <param name="q">Any System.Numerics Quaternion.</param>
		/// <returns>A StereoKit compatible quaternion.</returns>
		public static implicit operator Quat(Quaternion q) => new Quat(q.X, q.Y, q.Z, q.W);
		/// <summary>Allows implicit conversion from StereoKit.Quat to
		/// System.Numerics.Quaternion</summary>
		/// <param name="q">Any StereoKit.Quat.</param>
		/// <returns>A System.Numerics compatible quaternion.</returns>
		public static implicit operator Quaternion(Quat q) => q.q;

		public static Quat operator -(Quat a,  Quat b ) => NativeAPI.quat_difference(a, b);
		public static Quat operator *(Quat a,  Quat b ) => Quaternion.Multiply(a, b);
		public static Vec3 operator *(Quat a,  Vec3 pt) => Vector3.Transform(pt, a.q);
		public static Vec3 operator *(Vec3 pt, Quat a ) => Vector3.Transform(pt, a.q);

		/// <summary>This is the 'multiply by one!' of the quaternion
		/// rotation world. It's basically a default, no rotation quaternion.
		/// </summary>
		public static readonly Quat Identity = Quaternion.Identity;

		/// <summary>A normalized quaternion has the same orientation, and a 
		/// length of 1.</summary>
		public Quat Normalized => Quaternion.Normalize(q);

		/// <summary>The reverse rotation! If this quat goes from A to B, the 
		/// inverse will go from B to A.</summary>
		/// <returns>The inverse quaternion.</returns>
		public Quat Inverse => Quaternion.Inverse(q);

		/// <summary>A normalized quaternion has the same orientation, and a 
		/// length of 1.</summary>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void Normalize() => q = Quaternion.Normalize(q);

		/// <summary>Makes this Quat the reverse rotation! If this quat goes
		/// from A to B, the inverse will go from B to A.</summary>
		/// <returns>The inverse quaternion.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public void Invert() => q = Quaternion.Inverse(q);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Vec3 Rotate(Vec3 pt) => Vector3.Transform(pt, q);

		/// <summary>Creates a rotation that describes looking from a point,
		/// to another point! This is a great function for camera style
		/// rotation, or other facing behavior when you know where an object 
		/// is, and where you want it to look at. This rotation works best
		/// when applied to objects that face Vec3.Forward in their 
		/// resting/model space pose.</summary>
		/// <param name="lookFromPoint">Position of where the 'object' is.
		/// </param>
		/// <param name="lookAtPoint">Position of where the 'object' should
		/// be looking towards!</param>
		/// <param name="upDirection">Look From/At positions describe X and Y
		/// axis rotation well, but leave Z Axis/Roll undefined. Providing an
		/// upDirection vector helps to indicate roll around the From/At
		/// line. A common up direction would be (0,1,0), to prevent roll.
		/// </param>
		/// <returns>A rotation that describes looking from a point, towards
		/// another point.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quat LookAt(Vec3 lookFromPoint, Vec3 lookAtPoint, Vec3 upDirection) 
			=> NativeAPI.quat_lookat_up(lookFromPoint, lookAtPoint, upDirection);

		/// <summary>Creates a rotation that describes looking from a point,
		/// to another point! This is a great function for camera style
		/// rotation, or other facing behavior when you know where an object 
		/// is, and where you want it to look at. This rotation works best
		/// when applied to objects that face Vec3.Forward in their 
		/// resting/model space pose.
		/// 
		/// This overload automatically defines 'upDirection' as (0,1,0).
		/// This indicates the rotation should contain no roll around the Z
		/// axis, and is the most common way of using this type of rotation.
		/// </summary>
		/// <param name="lookFromPoint">Position of where the 'object' is.
		/// </param>
		/// <param name="lookAtPoint">Position of where the 'object' should
		/// be looking towards!</param>
		/// <returns>A rotation that describes looking from a point, towards
		/// another point.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quat LookAt(Vec3 lookFromPoint, Vec3 lookAtPoint) 
			=> NativeAPI.quat_lookat_up(lookFromPoint, lookAtPoint, Vec3.Up);

		/// <summary>Creates a rotation that describes looking towards a
		/// direction. This is great for quickly describing facing behavior!
		/// This rotation works best when applied to objects that face
		/// Vec3.Forward in their resting/model space pose.</summary>
		/// <param name="direction">Direction the rotation should be looking.
		/// Doesn't need to be normalized.</param>
		/// <returns>A rotation that describes looking towards a direction.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quat LookDir(Vec3 direction) 
			=> NativeAPI.quat_lookat(Vec3.Zero, direction);

		/// <summary>Creates a rotation that describes looking towards a
		/// direction. This is great for quickly describing facing behavior!
		/// This rotation works best when applied to objects that face
		/// Vec3.Forward in their resting/model space pose.</summary>
		/// <param name="x">X component of the direction the rotation should
		/// be looking. Doesn't need to be normalized.</param>
		/// <param name="y">Y component of the direction the rotation should
		/// be looking. Doesn't need to be normalized.</param>
		/// <param name="z">Z component of the direction the rotation should
		/// be looking. Doesn't need to be normalized.</param>
		/// <returns>A rotation that describes looking towards a direction.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quat LookDir(float x, float y, float z) 
			=> NativeAPI.quat_lookat(Vec3.Zero, new Vec3(x, y, z));

		/// <summary>This gives a relative rotation between the first and
		/// second quaternion rotations.
		/// Remember that order is important here!</summary>
		/// <param name="a">Starting rotation.</param>
		/// <param name="b">Ending rotation.</param>
		/// <returns>A rotation that will take a point from rotation a, to
		/// rotation b.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quat Difference(Quat a, Quat b) 
			=> NativeAPI.quat_difference(a, b);

		/// <summary>Creates a Roll/Pitch/Yaw rotation (applied in that
		/// order) from the provided angles in degrees!</summary>
		/// <param name="pitchXDeg">Pitch is rotation around the x axis,
		/// measured in degrees.</param>
		/// <param name="yawYDeg">Yaw is rotation around the y axis, measured
		/// in degrees.</param>
		/// <param name="rollZDeg">Roll is rotation around the z axis, 
		/// measured in degrees.</param>
		/// <returns>A quaternion representing the given Roll/Pitch/Yaw 
		/// rotation!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quat FromAngles(float pitchXDeg, float yawYDeg, float rollZDeg) 
			=> Quaternion.CreateFromYawPitchRoll(yawYDeg * Units.deg2rad, pitchXDeg * Units.deg2rad, rollZDeg * Units.deg2rad);
		/// <summary>Creates a Roll/Pitch/Yaw rotation (applied in that
		/// order) from the provided angles in degrees!</summary>
		/// <param name="pitchYawRollDeg">Pitch, yaw, and roll stored as
		/// X, Y, and Z in this Vector. Angle values are in degrees.</param>
		/// <returns>A quaternion representing the given Roll/Pitch/Yaw 
		/// rotation!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quat FromAngles(Vec3 pitchYawRollDeg) 
			=> Quaternion.CreateFromYawPitchRoll(pitchYawRollDeg.y * Units.deg2rad, pitchYawRollDeg.x * Units.deg2rad, pitchYawRollDeg.z * Units.deg2rad);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Vec3 Rotate(Quat q, Vec3 pt) 
			=> Vector3.Transform(pt, q);

		/// <summary>Spherical Linear intERPolation. Interpolates between two
		/// quaternions! Both Quats should be normalized/unit quaternions, or
		/// you may get unexpected results.</summary>
		/// <param name="a">Start quaternion, should be normalized/unit 
		/// length.</param>
		/// <param name="b">End quaternion, should be normalized/unit length.
		/// </param>
		/// <param name="slerp">The interpolation amount! This'll be a if 0, 
		/// and b if 1. Unclamped.</param>
		/// <returns>A blend between the two quaternions!</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static Quat Slerp(Quat a, Quat b, float slerp)
			=> Quaternion.Slerp(a,b, slerp);

		/// <summary>Mostly for debug purposes, this is a decent way to log or
		/// inspect the quaternion in debug mode. Looks like "[x, y, z, w]"
		/// </summary>
		/// <returns>A string that looks like "[x, y, z, w]"</returns>
		public override string ToString()
			=> string.Format("[{0:0.00}, {1:0.00}, {2:0.00}, {3:0.00}]", x, y, z, w);
	}
}
