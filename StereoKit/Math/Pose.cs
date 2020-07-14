using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>Pose represents a location and orientation in space, excluding scale!</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct Pose
	{
		/// <summary>Location of the pose.</summary>
		public Vec3 position;
		/// <summary>Orientation of the pose, stored as a rotation from Vec3.Forward.</summary>
		public Quat orientation;

		/// <summary>Calculates the forward direction from this pose. This is done by 
		/// multiplying the orientation with Vec3.Forward. Remember that Forward points
		/// down the -Z axis!</summary>
		public Vec3 Forward => orientation * Vec3.Forward;

		/// <summary>Calculates the right (+X) direction from this pose. This is done by 
		/// multiplying the orientation with Vec3.Right.</summary>
		public Vec3 Right => orientation * Vec3.Right;

		/// <summary>Calculates the up (+Y) direction from this pose. This is done by 
		/// multiplying the orientation with Vec3.Up.</summary>
		public Vec3 Up => orientation * Vec3.Up;

		/// <summary>Basic initialization constructor! Just copies in the provided values directly.</summary>
		/// <param name="position">Location of the pose.</param>
		/// <param name="orientation">Orientation of the pose, stored as a rotation from Vec3.Forward.</param>
		public Pose(Vec3 position, Quat orientation)
		{
			this.position    = position;
			this.orientation = orientation;
		}

		/// <summary>Basic initialization constructor! Just copies in the provided values directly.</summary>
		/// <param name="x">X location of the pose.</param>
		/// <param name="y">Y location of the pose.</param>
		/// <param name="z">Z location of the pose.</param>
		/// <param name="orientation">Orientation of the pose, stored as a rotation from Vec3.Forward.</param>
		public Pose(float x, float y, float z, Quat orientation)
		{
			this.position = new Vec3(x,y,z);
			this.orientation = orientation;
		}

		/// <summary>Converts this pose into a transform matrix, incorporating a provided scale value.</summary>
		/// <param name="scale">A scale vector! Vec3.One would be an identity scale.</param>
		/// <returns>A Matrix that transforms to the given pose.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Matrix ToMatrix(Vec3 scale) => NativeAPI.pose_matrix(this, scale);

		/// <summary>Converts this pose into a transform matrix, incorporating a provided scale value.</summary>
		/// <param name="scale">A uniform scale factor! 1 would be an identity scale.</param>
		/// <returns>A Matrix that transforms to the given pose.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Matrix ToMatrix(float scale) => NativeAPI.pose_matrix(this, new Vec3(scale, scale, scale));

		/// <summary>Converts this pose into a transform matrix.</summary>
		/// <returns>A Matrix that transforms to the given pose.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Matrix ToMatrix()           => NativeAPI.pose_matrix(this, Vec3.One);

		/// <summary>Interpolates between two poses! t is unclamped, so values outside of (0,1) will
		/// extrapolate their position.</summary>
		/// <param name="a">Starting pose, or percent == 0</param>
		/// <param name="b">Ending pose, or percent == 1</param>
		/// <param name="percent">A value usually 0->1 that tells the blend between a and b.</param>
		/// <returns>A new pose, blended between a and b based on percent!</returns>
		public static Pose Lerp(Pose a, Pose b, float percent)
			=> new Pose(Vec3.Lerp(a.position, b.position, percent), Quat.Slerp(a.orientation, b.orientation, percent));

		/// <summary>Converts a Windows Mirage spatial node GUID into a Pose
		/// based on its current position and rotation! Check 
		/// StereoKitApp.System.spatialBridge to see if this is available to
		/// use. Currently only on HoloLens, good for use with the Windows
		/// QR code package.</summary>
		/// <param name="spatialNodeGuid">A Windows Mirage spatial node GUID
		/// aquired from a windows MR API call.</param>
		/// <returns>A Pose representing the current orientation of the
		/// spatial node.</returns>
		public static Pose FromSpatialNode(Guid spatialNodeGuid)
			=> NativeAPI.pose_from_spatial(spatialNodeGuid.ToByteArray());

		public override string ToString()
			=> string.Format("{0}, {1}", position, Forward);
	};
}
