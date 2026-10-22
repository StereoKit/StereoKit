using StereoKit;
using System;
using System.Numerics;

namespace StereoKitUnitTest
{
	/// <summary>
    /// The intrinsic rotation order of the Euler.
	/// When converting to and from quaternions we need to know
	/// which order to apply the rotation. Pitch, then Yaw, 
	/// then Roll. OR Yaw, then Pitch, then Roll? Up to you!
    /// 
    /// This uses intrinsic rotation order.</summary>
    public enum RotOrder
	{
#pragma warning disable CS1591 // Missing XML comment for publicly visible type or member -- Disabled as its obvious
        YawPitchRoll = 0, // Default because its whats used in Quat.FromAngles() and Quaternion.FromYawPitchRoll()
		PitchYawRoll,
		PitchRollYaw,
		YawRollPitch,
        RollPitchYaw,
		RollYawPitch,
#pragma warning restore CS1591 // Missing XML comment for publicly visible type or member
	}

	/// <summary>Represent rotations in Euler angles.
	/// This is usually not preferred to using <see cref="Quat"/> but may be easier for some operations.
	/// All angles are specified in Pitch, Yaw and Roll.</summary>
    /// <remarks>In StereoKit this usually aligns with: X = Roll, Y = Yaw, Z = Pitch</remarks>
	public struct Euler
	{
		/// <summary>
        /// Order that the rotations are applied, defaults to <see cref="RotOrder.RollPitchYaw"/>.
        /// Check docs for <seealso cref="RotOrder"/> for more info.
		/// </summary>
        public RotOrder RotationOrder;

		/// <summary>
        /// Pitch rotation in Degrees
		/// </summary>
		/// <remarks>
        /// See the <see cref="RotationOrder"/> for which order this is applied. Its not always RollYawPitch!
		/// </remarks>
		public float XPitch { get; set; }

		/// <summary>
        /// Yaw rotation in Degrees
		/// </summary>
		/// <remarks>
        /// See the <see cref="RotationOrder"/> for which order this is applied. Its not always RollYawPitch!
		/// </remarks>
		public float YYaw { get; set; }

		/// <summary>
        /// Roll rotation in Degrees
		/// </summary>
		/// <remarks>
        /// See the <see cref="RotationOrder"/> for which order this is applied. Its not always RollYawPitch!
		/// </remarks>
		public float ZRoll { get; set; }

		/// <summary>
        /// Setup a Euler, which will apply rotations in the the specified <see cref="RotOrder"/>
		/// </summary>
        /// <param name="eulerRotationOrder">The rotation order</param>
        public Euler(RotOrder eulerRotationOrder)
		{
            RotationOrder = eulerRotationOrder;
			XPitch = 0;
			YYaw = 0;
			ZRoll = 0;
		}

        /// <inheritdoc cref="Euler(RotOrder)"/>
        /// <param name="pitch">Pitch in degrees (X axis rotation)</param>
        /// <param name="yaw">Yaw in degrees (Y axis rotation)</param>
        /// <param name="roll">Roll in degrees (Z axis rotation)</param>
        public Euler(float pitch, float yaw, float roll, RotOrder eulerRotationOrder)
            : this(eulerRotationOrder)
		{
			XPitch = pitch;
			YYaw = yaw;
			ZRoll = roll;
		}

		/// <summary>
		/// Convert a Quaternion to an euler representation.
		/// Specify an Euler Order to specify which order the rotations are applied.
		/// </summary>
		/// <returns>A new Euler</returns>
        public static Euler FromQuat(Quat quat, RotOrder order)
		{
			
			// Creates a row major matrix
			var m = Matrix4x4.CreateFromQuaternion(quat.Normalized);
			// Ported as is, from from threejs: https://github.com/mrdoob/three.js/blob/4503ef10b81a00f5c6c64fe9a856881ee31fe6a3/src/math/Euler.js#L105

			// Assumes the upper 3x3 of m is a pure rotation matrix (i.e, unscaled)
			// ALSO transposes the matrix from dotnet RowMajor Matrix4x4 to ThreeJS ColumnMajor Matrix4x4
			// csharpier-ignore-start
			float m11 = m.M11, m12 = m.M21, m13 = m.M31;
			float m21 = m.M12, m22 = m.M22, m23 = m.M32;
			float m31 = m.M13, m32 = m.M23, m33 = m.M33;
			// csharpier-ignore-end

			var euler = new Euler(order);

            // This code converts the quaternion to it Intrinsic Euler rotations.
			switch (order)
			{
                case RotOrder.PitchYawRoll:

					euler.YYaw = (float)Math.Asin(SKMath.Clamp(m13, -1, 1));

					if (Math.Abs(m13) < 0.9999999)
					{
						euler.XPitch = (float)Math.Atan2(-m23, m33);
						euler.ZRoll = (float)Math.Atan2(-m12, m11);
					}
					else
					{
						euler.XPitch = (float)Math.Atan2(m32, m22);
						euler.ZRoll = 0;
					}

					break;

                case RotOrder.YawPitchRoll:

					euler.XPitch = (float)Math.Asin(-SKMath.Clamp(m23, -1, 1));

					if (Math.Abs(m23) < 0.9999999)
					{
						euler.YYaw = (float)Math.Atan2(m13, m33);
						euler.ZRoll = (float)Math.Atan2(m21, m22);
					}
					else
					{
						euler.YYaw = (float)Math.Atan2(-m31, m11);
						euler.ZRoll = 0;
					}

					break;

                case RotOrder.RollPitchYaw:

					euler.XPitch = (float)Math.Asin(SKMath.Clamp(m32, -1, 1));

					if (Math.Abs(m32) < 0.9999999)
					{
						euler.YYaw = (float)Math.Atan2(-m31, m33);
						euler.ZRoll = (float)Math.Atan2(-m12, m22);
					}
					else
					{
						euler.YYaw = 0;
						euler.ZRoll = (float)Math.Atan2(m21, m11);
					}

					break;

                case RotOrder.RollYawPitch:

					euler.YYaw = (float)Math.Asin(-SKMath.Clamp(m31, -1, 1));

					if (Math.Abs(m31) < 0.9999999)
					{
						euler.XPitch = (float)Math.Atan2(m32, m33);
						euler.ZRoll = (float)Math.Atan2(m21, m11);
					}
					else
					{
						euler.XPitch = 0;
						euler.ZRoll = (float)Math.Atan2(-m12, m22);
					}

					break;

                case RotOrder.YawRollPitch:

					euler.ZRoll = (float)Math.Asin(SKMath.Clamp(m21, -1, 1));

					if (Math.Abs(m21) < 0.9999999)
					{
						euler.XPitch = (float)Math.Atan2(-m23, m22);
						euler.YYaw = (float)Math.Atan2(-m31, m11);
					}
					else
					{
						euler.XPitch = 0;
						euler.YYaw = (float)Math.Atan2(m13, m33);
					}

					break;

                case RotOrder.PitchRollYaw:

					euler.ZRoll = (float)Math.Asin(-SKMath.Clamp(m12, -1, 1));

					if (Math.Abs(m12) < 0.9999999)
					{
						euler.XPitch = (float)Math.Atan2(m32, m22);
						euler.YYaw = (float)Math.Atan2(m13, m11);
					}
					else
					{
						euler.XPitch = (float)Math.Atan2(-m23, m33);
						euler.YYaw = 0;
					}

					break;

				default:
					throw new ArgumentOutOfRangeException(nameof(order));
			}

            // Convert from radians to degrees
            euler.XPitch *= Units.rad2deg;
            euler.YYaw *= Units.rad2deg;
            euler.ZRoll *= Units.rad2deg;
			return euler;
		}

		/// <summary>
		/// Convert this Euler to a Quaternion, this is needed to use it for rotations etc.
		/// </summary>
		/// <returns>A new Quaternion applied in the intended order</returns>
		public Quat ToQuat()
		{
            // Convert from degrees to radians, as the below code needs radian inputs.
			// csharpier-ignore
            float pitch = XPitch * Units.deg2rad, yaw = YYaw * Units.deg2rad, roll = ZRoll * Units.deg2rad;
            RotOrder order = this.RotationOrder;

			// Ported from from https://github.com/mrdoob/three.js/blob/4503ef10b81a00f5c6c64fe9a856881ee31fe6a3/src/math/Quaternion.js#L201

			// Which again attributes:
			// http://www.mathworks.com/matlabcentral/fileexchange/20696-function-to-convert-between-dcm-euler-angles-quaternions-and-euler-vectors/content/SpinCalc.m

            // This code converts the Intrinsic Euler rotations to quaternions.
            // This actually means that we apply the rotations in inverse order (Extrinsic),
            // because the math for Extrinsic is faster.
            double cp = Math.Cos(pitch / 2);
            double cy = Math.Cos(yaw / 2);
            double cr = Math.Cos(roll / 2);

            double sp = Math.Sin(pitch / 2);
            double sy = Math.Sin(yaw / 2);
            double sr = Math.Sin(roll / 2);
			var quat = new Quat();
			switch (order)
			{
                case RotOrder.PitchYawRoll:
                    quat.x = (float)(cy * sp * cr + sy * cp * sr);
                    quat.y = (float)(sy * cp * cr - cy * sp * sr);
                    quat.z = (float)(cy * cp * sr + sy * sp * cr);
                    quat.w = (float)(cy * cp * cr - sy * sp * sr);
					break;

                case RotOrder.YawPitchRoll:
                    quat.x = (float)(sp * cy * cr + cp * sy * sr);
                    quat.y = (float)(cp * sy * cr - sp * cy * sr);
                    quat.z = (float)(cp * cy * sr - sp * sy * cr);
                    quat.w = (float)(cp * cy * cr + sp * sy * sr);
					break;

                case RotOrder.RollPitchYaw:
                    quat.x = (float)(sp * cy * cr - cp * sy * sr);
                    quat.y = (float)(cp * sy * cr + sp * cy * sr);
                    quat.z = (float)(cp * cy * sr + sp * sy * cr);
                    quat.w = (float)(cp * cy * cr - sp * sy * sr);
					break;

                case RotOrder.RollYawPitch:
                    quat.x = (float)(sp * cy * cr - cp * sy * sr);
                    quat.y = (float)(cp * sy * cr + sp * cy * sr);
                    quat.z = (float)(cp * cy * sr - sp * sy * cr);
                    quat.w = (float)(cp * cy * cr + sp * sy * sr);
					break;

                case RotOrder.YawRollPitch:
                    quat.x = (float)(sp * cy * cr + cp * sy * sr);
                    quat.y = (float)(cp * sy * cr + sp * cy * sr);
                    quat.z = (float)(cp * cy * sr - sp * sy * cr);
                    quat.w = (float)(cp * cy * cr - sp * sy * sr);
					break;

                case RotOrder.PitchRollYaw:
                    quat.x = (float)(sp * cy * cr - cp * sy * sr);
                    quat.y = (float)(cp * sy * cr - sp * cy * sr);
                    quat.z = (float)(cp * cy * sr + sp * sy * cr);
                    quat.w = (float)(cp * cy * cr + sp * sy * sr);
					break;

				default:
					throw new ArgumentOutOfRangeException(
						nameof(order),
						order,
						"Got an invalid order"
					);
			}

			return quat;
		}

		/// <summary>Pretty-prints this Euler to a human readable debug format.
		/// Printed values are in Degrees
		/// [Pitch: pitch, Yaw: yaw, Roll: roll] Order: order</summary>
		public override string ToString()
		{
            return $"[Pitch: {XPitch}, Yaw: {YYaw}, Roll: {ZRoll}] Order: {RotationOrder}";
		}
	}
}
