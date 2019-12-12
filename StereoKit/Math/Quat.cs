using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace StereoKit
{
    /// <summary>Quaternions are efficient and robust mathematical objects for 
    /// representing rotations! Understanding the details of how a quaterion works
    /// is not generally necessary for using them effectively, so don't worry too
    /// much if they seem weird to you. They're weird to me too.
    /// 
    /// If you're interested in learning the details though, 3Blue1Brown and Ben Eater
    /// have an [excellent interactive lesson](https://eater.net/quaternions) about them!</summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct Quat
    {
        public float x, y, z, w;

        /// <summary>This is the 'multiply by one!' of the quaternion rotation world. It's 
        /// basically a default, no rotation quaternion.</summary>
        public static readonly Quat Identity = new Quat(0, 0, 0, 1);

        /// <summary>You may want to use static creation methods, like Quat.LookAt, 
        /// or Quat.Identity instead of this one! Unless you know what you're doing.</summary>
        /// <param name="x">X component of the Quat.</param>
        /// <param name="y">Y component of the Quat.</param>
        /// <param name="z">Z component of the Quat.</param>
        /// <param name="w">W component of the Quat.</param>
        public Quat(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public override string ToString()
        {
            return string.Format("<{0:0.00}, {1:0.00}, {2:0.00}, {3:0.00}>", x, y, z, w);
        }

        /// <summary>A normalized quaternion has the same orientation, and a length of 1.</summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public void Normalize()
        {
            this = NativeAPI.quat_normalize(this);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quat operator -(Quat a, Quat b) => NativeAPI.quat_difference(a,b);
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quat operator *(Quat a, Quat b) => NativeAPI.quat_mul(a, b);
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vec3 operator *(Quat a, Vec3 pt) => NativeAPI.quat_mul_vec(a, pt);

        /// <summary>Creates a rotation from a resting direction, to a direction indicated
        /// by the direction of the two vectors provided to the function! This is a great
        /// function for camera style rotation, when you know where a camera is, and where
        /// you want to look at. This prevents roll on the Z axis, Up is always (0,1,0).</summary>
        /// <param name="from">Where the object is.</param>
        /// <param name="to">Where the object should be looking!</param>
        /// <returns>A rotation from resting, to the look direction of the parameters.</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quat LookAt    (Vec3 from, Vec3 to) => NativeAPI.quat_lookat(from, to);

        /// <summary>Creates a rotation from a resting direction, to a direction indicated
        /// by the direction of the two vectors provided to the function! This is a great
        /// function for camera style rotation, when you know where a camera is, and where
        /// you want to look at. This prevents roll on the Z axis, Up is always (0,1,0).</summary>
        /// <param name="from">Where the object is.</param>
        /// <param name="to">Where the object should be looking!</param>
        /// <param name="up">Which direction is up from this perspective?</param>
        /// <returns>A rotation from resting, to the look direction of the parameters.</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quat LookAt(Vec3 from, Vec3 to, Vec3 up) => NativeAPI.quat_lookat_up(from, to, up);

        /// <summary>Creates a rotation from a resting direction, to the given direction! This 
        /// prevents roll on the Z axis, Up is always (0,1,0)</summary>
        /// <param name="direction">Direction the rotation should be looking. Doesn't need to be normalized.</param>
        /// <returns>A rotation from resting, to the given direction.</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quat LookDir   (Vec3 direction) => NativeAPI.quat_lookat(Vec3.Zero, direction);

        /// <summary>Creates a rotation from a resting direction, to the given direction! This 
        /// prevents roll on the Z axis, Up is always (0,1,0)</summary>
        /// <param name="x">X component of the direction the rotation should be looking. Doesn't need to be normalized.</param>
        /// <param name="y">Y component of the direction the rotation should be looking. Doesn't need to be normalized.</param>
        /// <param name="z">Z component of the direction the rotation should be looking. Doesn't need to be normalized.</param>
        /// <returns>A rotation from resting, to the given direction.</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quat LookDir(float x, float y, float z) => NativeAPI.quat_lookat(Vec3.Zero, new Vec3(x,y,z));

        /// <summary>This gives a relative rotation between the first and second quaternion rotations.
        /// Remember that order is important here!</summary>
        /// <param name="a">Starting rotation.</param>
        /// <param name="b">Ending rotation.</param>
        /// <returns>A rotation that will take a point from rotation a, to rotation b.</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quat Difference(Quat a, Quat b) => NativeAPI.quat_difference(a, b);

        /// <summary>Creates a Roll/Pitch/Yaw rotation from the provided angles in degrees!</summary>
        /// <param name="pitchXDeg">Pitch is rotation around the x axis, measured in degrees.</param>
        /// <param name="yawYDeg">Yaw is rotation around the y axis, measured in degrees.</param>
        /// <param name="rollZDeg">Roll is rotation around the z axis, measured in degrees.</param>
        /// <returns></returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quat FromAngles(float pitchXDeg, float yawYDeg, float rollZDeg) => NativeAPI.quat_from_angles(pitchXDeg, yawYDeg, rollZDeg);

        /// <summary>Spherical Linear intERPolation. Interpolates between two quaternions! Both
        /// Quats should be normalized/unit quaternions, or you may get unexpected results.</summary>
        /// <param name="a">Start quaternion, should be normalized/unit length.</param>
        /// <param name="b">End quaternion, should be normalized/unit length.</param>
        /// <param name="slerp">The interpolation amount! This'll be a if 0, and b if 1. Unclamped.</param>
        /// <returns>A blend between the two quaternions!</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quat Slerp(Quat a, Quat b, float slerp) => NativeAPI.quat_slerp(a, b, slerp);

    }
}
