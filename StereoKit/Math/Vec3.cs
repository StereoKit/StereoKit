using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace StereoKit
{
    /// <summary>A vector with 3 components: x, y, z. This can represent a point in space,
    /// a directional vector, or any other sort of value with 3 dimensions to it!
    /// 
    /// StereoKit uses a right-handed coordinate system, where +x is to the right, +y is
    /// upwards, and -z is forward.</summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct Vec3
    {
        public float x, y, z;

        /// <summary>Magnitude is the length of the vector! Or the distance from the origin
        /// to this point. Uses Math.Sqrt, so it's not dirt cheap or anything.</summary>
        public float Magnitude
        { 
            get => SKMath.Sqrt(x*x + y*y + z*z);
        }
        /// <summary>This is the squared magnitude of the vector! It skips the Sqrt call, and
        /// just gives you the squared version for speedy calculations that can work with it squared.</summary>
        public float MagnitudeSq {
            get => x*x + y*y + z*z;
        }

        /// <summary>Creates a vector from x, y, and z values! StereoKit uses a right-handed metric
        /// coordinate system, where +x is to the right, +y is upwards, and -z is forward.</summary>
        /// <param name="x">The x axis.</param>
        /// <param name="y">The y axis.</param>
        /// <param name="z">The z axis.</param>
        public Vec3(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        /// <summary>Creates a normalized vector (vector with a length of 1) from the
        /// current vector. Will not work properly if the vector has a length of zero.</summary>
        /// <returns>The normalized (length of 1) vector!</returns>
        public Vec3 Normalized()
        {
            float mag = SKMath.Sqrt(x*x+y*y+z*z);
            return new Vec3(x/mag, y/mag, z/mag);
        }

        public override string ToString()
        {
            return string.Format("<{0:0.00}, {1:0.00}, {2:0.00}>", x, y, z);
        }

        /// <summary>Shorthand for a vector where all values are 0! Same as `new Vec3(0,0,0)`.</summary>
        public static readonly Vec3 Zero    = new Vec3(0,0,0);
        /// <summary>Shorthand for a vector where all values are 1! Same as `new Vec3(1,1,1)`.</summary>
        public static readonly Vec3 One     = new Vec3(1,1,1);
        /// <summary>A vector representing the up axis. In StereoKit, this is the same as `new Vec3(0,1,0)`.</summary>
        public static readonly Vec3 Up      = new Vec3(0,1,0);
        /// <summary>StereoKit uses a right-handed coordinate system, which means that forward is 
        /// looking down the -Z axis! This value is the same as `new Vec3(0,0,-1)`</summary>
        public static readonly Vec3 Forward = new Vec3(0,0,-1);
        /// <summary>When looking forward, this is the direction to the right! In StereoKit, this is the same as `new Vec3(1,0,0)`</summary>
        public static readonly Vec3 Right   = new Vec3(1,0,0);
        public static Vec3 operator +(Vec3 a, Vec3 b)  { return new Vec3(a.x + b.x, a.y + b.y, a.z + b.z); }
        public static Vec3 operator -(Vec3 a, Vec3 b)  { return new Vec3(a.x - b.x, a.y - b.y, a.z - b.z); }
        public static Vec3 operator -(Vec3 a)          { return new Vec3(-a.x, -a.y, -a.z); }
        public static Vec3 operator *(Vec3 a, float b) { return new Vec3(a.x * b, a.y * b, a.z * b); }
        public static Vec3 operator /(Vec3 a, float b) { return new Vec3(a.x / b, a.y / b, a.z / b); }

        /// <summary>Calculates the distance between two points in space! Make sure they're in the
        /// same coordinate space! Uses a Sqrt, so it's not blazing fast, prefer DistanceSq when possible.</summary>
        /// <param name="a">The first point/</param>
        /// <param name="b">And the second point!</param>
        /// <returns>Distance between the two points.</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Distance(in Vec3 a, in Vec3 b)
            => SKMath.Sqrt(a.x*b.x + a.y*b.y + a.z*b.z);

        /// <summary>Calculates the distance between two points in space, but leaves them squared! Make 
        /// sure they're in the same coordinate space! This is a fast function :)</summary>
        /// <param name="a">The first point/</param>
        /// <param name="b">And the second point!</param>
        /// <returns>Distance between the two points, but squared!</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float DistanceSq(in Vec3 a, in Vec3 b)
            => a.x*b.x + a.y*b.y + a.z*b.z;

        /// <summary>The dot product is an extremely useful operation! One major use is to determine
        /// how similar two vectors are. If the vectors are Unit vectors (magnitude/length of 1), then 
        /// the result will be 1 if the vectors are the same, -1 if they're opposite, and a gradient 
        /// in-between with 0 being perpendicular.</summary>
        /// <param name="a">First vector.</param>
        /// <param name="b">Second vector.</param>
        /// <returns>The dot product!</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Dot(in Vec3 a, in Vec3 b)
            => a.x*b.x + a.y*b.y + a.z*b.z;

        /// <summary>The cross product of two vectors!</summary>
        /// <param name="a">First vector!</param>
        /// <param name="b">Second vector!</param>
        /// <returns>Result is -not- a unit vector, even if both 'a'
        /// and 'b' are unit vectors.</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vec3 Cross(in Vec3 a, in Vec3 b)
            => NativeAPI.vec3_cross(a, b);

        /// <summary>Exactly the same as Vec3.Cross, but has some naming memnonics for getting the order
        /// right when trying to find a perpendicular vector using the cross product. This'll also make it
        /// mor obvious to read if that's what you're actually going for when crossing vectors!
        /// 
        /// If you consider a forward vector and an up vector, then the direction to the right is pretty
        /// trivial to imagine in relation to those vectors!</summary>
        /// <param name="forward">What way are you facing?</param>
        /// <param name="up">What direction is up?</param>
        /// <returns>Your direction to the right! Result is -not- a unit vector, even if both 'forward'
        /// and 'up' are unit vectors.</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vec3 PerpendicularRight(in Vec3 forward, in Vec3 up)
            => NativeAPI.vec3_cross(forward, up);

        /// <summary>Creates a vector that points out at the given 2D angle! This
        /// creates the vector on the XZ plane, and allows you to specify a constant
        /// y value.</summary>
        /// <param name="angleDeg">Angle in degrees, starting from (1,0) at 0, and
        /// continuing to (0,1) at 90.</param>
        /// <param name="y">A constant value you can assign to the resulting vector's
        /// y component.</param>
        /// <returns>A Vector pointing at the given angle! If y is zero, this will be
        /// a normalized vector (vector with a length of 1).</returns>
        public static Vec3 AngleXZ(float angleDeg, float y = 0)
            => new Vec3(SKMath.Cos(angleDeg*Units.deg2rad), y, SKMath.Sin(angleDeg*Units.deg2rad));

        /// <summary>Creates a vector that points out at the given 2D angle! This
        /// creates the vector on the XY plane, and allows you to specify a constant
        /// z value.</summary>
        /// <param name="angleDeg">Angle in degrees, starting from (1,0) at 0, and
        /// continuing to (0,1) at 90.</param>
        /// <param name="z">A constant value you can assign to the resulting vector's
        /// z component.</param>
        /// <returns>A vector pointing at the given angle! If z is zero, this will be
        /// a normalized vector (vector with a length of 1).</returns>
        public static Vec3 AngleXY(float angleDeg, float z = 0)
            => new Vec3(SKMath.Cos(angleDeg*Units.deg2rad), SKMath.Sin(angleDeg*Units.deg2rad), z);
    }
}
