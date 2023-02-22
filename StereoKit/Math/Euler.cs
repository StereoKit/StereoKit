using StereoKit;
using System;
using System.Numerics;

namespace StereoKitUnitTest
{
	/// <summary>
	/// The rotation order of the Euler.
	/// When converting to and from quaternions we need to know
	/// which order to apply the rotation. Pitch, then Yaw, 
	/// then Roll. OR Yaw, then Pitch, then Roll? Up to you!
	/// </summary>
	public enum EulerRotationOrder
	{
#pragma warning disable CS1591 // Missing XML comment for publicly visible type or member -- Disabled as its obvious
		RollPitchYaw = 0, // Default because its whats used internally in Quat.FromAngles() and Quaternion.FromYawPitchRoll()
		PitchYawRoll,
		PitchRollYaw,
		YawRollPitch,
		YawPitchRoll,
		RollYawPitch,
#pragma warning restore CS1591 // Missing XML comment for publicly visible type or member
	}

	/// <summary>Represent rotations in Euler angles.
	/// This is usually not preferred to using <see cref="Quat"/> but may be easier for some operations.
	/// All angles are specified in Pitch, Yaw and Roll.</summary>
	/// <remarks>
	/// <para>
	/// In StereoKit this usually aligns with: 
	///		X = Roll,
	///		Y = Yaw,
	///		Z = Pitch
	///	</para>
	/// </remarks>
	public struct Euler
	{
		const float RadToDeg = 1 / (SKMath.Pi / 180);

		/// <summary>
		/// Order that the rotations are applied, defaults to <see cref="EulerRotationOrder.RollPitchYaw"/>.
		/// Check docs for <seealso cref="EulerRotationOrder"/> for more info.
		/// </summary>
		public EulerRotationOrder Order;

		/// <summary>
		/// Pitch rotation in Radians
		/// </summary>
		/// <remarks>
		/// See the <see cref="Order"/> for which order this is applied. Its not always RollYawPitch!
		/// </remarks>
		public float XPitch { get; set; }
		public float XPitchDeg => XPitch * RadToDeg;

		/// <summary>
		/// Yaw rotation in Radians
		/// </summary>
		/// <remarks>
		/// See the <see cref="Order"/> for which order this is applied. Its not always RollYawPitch!
		/// </remarks>
		public float YYaw { get; set; }
		public float YYawDeg => YYaw * RadToDeg;

		/// <summary>
		/// Roll rotation in Radians
		/// </summary>
		/// <remarks>
		/// See the <see cref="Order"/> for which order this is applied. Its not always RollYawPitch!
		/// </remarks>
		public float ZRoll { get; set; }
		public float ZRollDeg => ZRoll * RadToDeg;

		/// <summary>
		/// Setup a Euler, which will apply rotations in the the specified <see cref="EulerRotationOrder"/>
		/// </summary>
		public Euler(EulerRotationOrder order)
		{
			Order = order;
			XPitch = 0;
			YYaw = 0;
			ZRoll = 0;
		}

		/// <inheritdoc cref="Euler(EulerRotationOrder)"/>
		/// <param name="pitch">Pitch in radians (X axis rotation)</param>
		/// <param name="yaw">Yaw in radians (Y axis rotation)</param>
		/// <param name="roll">Roll in radians (Z axis rotation)</param>
		public Euler(float pitch, float yaw, float roll, EulerRotationOrder order)
			: this(order)
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
		public static Euler FromQuat(Quat quat, EulerRotationOrder order)
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

			switch (order)
			{
				case EulerRotationOrder.PitchYawRoll:

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

				case EulerRotationOrder.YawPitchRoll:

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

				case EulerRotationOrder.RollPitchYaw:

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

				case EulerRotationOrder.RollYawPitch:

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

				case EulerRotationOrder.YawRollPitch:

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

				case EulerRotationOrder.PitchRollYaw:

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

			return euler;
		}

		/// <summary>
		/// Convert this Euler to a Quaternion, this is needed to use it for rotations etc.
		/// </summary>
		/// <returns>A new Quaternion applied in the intended order</returns>
		public Quat ToQuat()
		{
			// csharpier-ignore
			float x = this.XPitch, y = this.YYaw, z = this.ZRoll;
			EulerRotationOrder order = this.Order;

			// Ported from from https://github.com/mrdoob/three.js/blob/4503ef10b81a00f5c6c64fe9a856881ee31fe6a3/src/math/Quaternion.js#L201

			// Which again attributes:
			// http://www.mathworks.com/matlabcentral/fileexchange/20696-function-to-convert-between-dcm-euler-angles-quaternions-and-euler-vectors/content/SpinCalc.m

			double c1 = Math.Cos(x / 2);
			double c2 = Math.Cos(y / 2);
			double c3 = Math.Cos(z / 2);

			double s1 = Math.Sin(x / 2);
			double s2 = Math.Sin(y / 2);
			double s3 = Math.Sin(z / 2);
			var quat = new Quat();
			switch (order)
			{
				case EulerRotationOrder.PitchYawRoll:
					quat.x = (float)(s1 * c2 * c3 + c1 * s2 * s3);
					quat.y = (float)(c1 * s2 * c3 - s1 * c2 * s3);
					quat.z = (float)(c1 * c2 * s3 + s1 * s2 * c3);
					quat.w = (float)(c1 * c2 * c3 - s1 * s2 * s3);
					break;

				case EulerRotationOrder.YawPitchRoll:
					quat.x = (float)(s1 * c2 * c3 + c1 * s2 * s3);
					quat.y = (float)(c1 * s2 * c3 - s1 * c2 * s3);
					quat.z = (float)(c1 * c2 * s3 - s1 * s2 * c3);
					quat.w = (float)(c1 * c2 * c3 + s1 * s2 * s3);
					break;

				case EulerRotationOrder.RollPitchYaw:
					quat.x = (float)(s1 * c2 * c3 - c1 * s2 * s3);
					quat.y = (float)(c1 * s2 * c3 + s1 * c2 * s3);
					quat.z = (float)(c1 * c2 * s3 + s1 * s2 * c3);
					quat.w = (float)(c1 * c2 * c3 - s1 * s2 * s3);
					break;

				case EulerRotationOrder.RollYawPitch:
					quat.x = (float)(s1 * c2 * c3 - c1 * s2 * s3);
					quat.y = (float)(c1 * s2 * c3 + s1 * c2 * s3);
					quat.z = (float)(c1 * c2 * s3 - s1 * s2 * c3);
					quat.w = (float)(c1 * c2 * c3 + s1 * s2 * s3);
					break;

				case EulerRotationOrder.YawRollPitch:
					quat.x = (float)(s1 * c2 * c3 + c1 * s2 * s3);
					quat.y = (float)(c1 * s2 * c3 + s1 * c2 * s3);
					quat.z = (float)(c1 * c2 * s3 - s1 * s2 * c3);
					quat.w = (float)(c1 * c2 * c3 - s1 * s2 * s3);
					break;

				case EulerRotationOrder.PitchRollYaw:
					quat.x = (float)(s1 * c2 * c3 - c1 * s2 * s3);
					quat.y = (float)(c1 * s2 * c3 - s1 * c2 * s3);
					quat.z = (float)(c1 * c2 * s3 + s1 * s2 * c3);
					quat.w = (float)(c1 * c2 * c3 + s1 * s2 * s3);
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
			return $"(deg)[Pitch: {XPitchDeg}, Yaw: {YYawDeg}, Roll: {ZRollDeg}] Order: {Order}";
		}
	}
}
