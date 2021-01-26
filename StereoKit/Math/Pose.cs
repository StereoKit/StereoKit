using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>Pose represents a location and orientation in space, excluding scale!</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct Pose
	{
		/// <summary>Location of the pose.</summary>
		public Vector3 position;
		/// <summary>Orientation of the pose, stored as a rotation from Vec3.Forward.</summary>
		public Quaternion orientation;

		/// <summary>Calculates the forward direction from this pose. This is done by 
		/// multiplying the orientation with Vec3.Forward. Remember that Forward points
		/// down the -Z axis!</summary>
		public Vector3 Forward => Vec3.Forward.Rotate(orientation);

		/// <summary>Calculates the right (+X) direction from this pose. This is done by 
		/// multiplying the orientation with Vec3.Right.</summary>
		public Vector3 Right => Vec3.Right.Rotate(orientation);

		/// <summary>Calculates the up (+Y) direction from this pose. This is done by 
		/// multiplying the orientation with Vec3.Up.</summary>
		public Vector3 Up => Vec3.Up.Rotate(orientation);

		/// <summary>This creates a ray starting at the Pose's position, and
		/// pointing in the 'Forward' direction. The Ray direction is a unit
		/// vector/normalized. </summary>
		public Ray Ray => new Ray(position, Forward);

		/// <summary>Basic initialization constructor! Just copies in the provided values directly.</summary>
		/// <param name="position">Location of the pose.</param>
		/// <param name="orientation">Orientation of the pose, stored as a rotation from Vec3.Forward.</param>
		public Pose(Vector3 position, Quaternion orientation)
		{
			this.position    = position;
			this.orientation = orientation;
		}

		/// <summary>Basic initialization constructor! Just copies in the provided values directly.</summary>
		/// <param name="x">X location of the pose.</param>
		/// <param name="y">Y location of the pose.</param>
		/// <param name="z">Z location of the pose.</param>
		/// <param name="orientation">Orientation of the pose, stored as a rotation from Vec3.Forward.</param>
		public Pose(float x, float y, float z, Quaternion orientation)
		{
			this.position = new Vector3(x,y,z);
			this.orientation = orientation;
		}

		/// <summary>Converts this pose into a transform matrix, incorporating a provided scale value.</summary>
		/// <param name="scale">A scale vector! Vec3.One would be an identity scale.</param>
		/// <returns>A Matrix that transforms to the given pose.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Matrix4x4 ToMatrix(Vector3 scale) => NativeAPI.pose_matrix(this, scale);

		/// <summary>Converts this pose into a transform matrix, incorporating a provided scale value.</summary>
		/// <param name="scale">A uniform scale factor! 1 would be an identity scale.</param>
		/// <returns>A Matrix that transforms to the given pose.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Matrix4x4 ToMatrix(float scale) => NativeAPI.pose_matrix(this, new Vector3(scale, scale, scale));

		/// <summary>Converts this pose into a transform matrix.</summary>
		/// <returns>A Matrix that transforms to the given pose.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Matrix4x4 ToMatrix()           => NativeAPI.pose_matrix(this, Vector3.One);

		/// <summary>Interpolates between two poses! t is unclamped, so values outside of (0,1) will
		/// extrapolate their position.</summary>
		/// <param name="a">Starting pose, or percent == 0</param>
		/// <param name="b">Ending pose, or percent == 1</param>
		/// <param name="percent">A value usually 0->1 that tells the blend between a and b.</param>
		/// <returns>A new pose, blended between a and b based on percent!</returns>
		public static Pose Lerp(Pose a, Pose b, float percent)
			=> new Pose(Vector3.Lerp(a.position, b.position, percent), Quaternion.Slerp(a.orientation, b.orientation, percent));

		public override string ToString()
			=> string.Format("{0}, {1}", position, Forward);
	};
}
