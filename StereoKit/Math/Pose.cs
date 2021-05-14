using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>Pose represents a location and orientation in space, 
	/// excluding scale! CAUTION: the default value of a Pose includes a 
	/// completely zero Quat, which can cause problems. Use `Pose.Identity` 
	/// instead of `new Pose()` for creating a default pose.</summary>
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
		public Vec3 Forward => Vec3.Forward * orientation;

		/// <summary>Calculates the right (+X) direction from this pose. This is done by 
		/// multiplying the orientation with Vec3.Right.</summary>
		public Vec3 Right => Vec3.Right * orientation;

		/// <summary>Calculates the up (+Y) direction from this pose. This is done by 
		/// multiplying the orientation with Vec3.Up.</summary>
		public Vec3 Up => Vec3.Up * orientation;

		/// <summary>This creates a ray starting at the Pose's position, and
		/// pointing in the 'Forward' direction. The Ray direction is a unit
		/// vector/normalized. </summary>
		public Ray Ray => new Ray(position, Forward);

		/// <summary>A default, empty pose. Positioned at zero, and using
		/// Quat.Identity for orientation.</summary>
		public static Pose Identity
			=> new Pose(Vec3.Zero, Quat.Identity);

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
		public Matrix ToMatrix(Vec3 scale) 
			=> Matrix.TRS(position, orientation, scale);

		/// <summary>Converts this pose into a transform matrix, incorporating a provided scale value.</summary>
		/// <param name="scale">A uniform scale factor! 1 would be an identity scale.</param>
		/// <returns>A Matrix that transforms to the given pose.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Matrix ToMatrix(float scale)
			=> Matrix.TRS(position, orientation, scale);

		/// <summary>Converts this pose into a transform matrix.</summary>
		/// <returns>A Matrix that transforms to the given pose.</returns>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public Matrix ToMatrix() 
			=> Matrix.TR(position, orientation);

		/// <summary>Interpolates between two poses! t is unclamped, so values outside of (0,1) will
		/// extrapolate their position.</summary>
		/// <param name="a">Starting pose, or percent == 0</param>
		/// <param name="b">Ending pose, or percent == 1</param>
		/// <param name="percent">A value usually 0->1 that tells the blend between a and b.</param>
		/// <returns>A new pose, blended between a and b based on percent!</returns>
		public static Pose Lerp(Pose a, Pose b, float percent)
			=> new Pose(Vec3.Lerp(a.position, b.position, percent), Quat.Slerp(a.orientation, b.orientation, percent));

		public override string ToString()
			=> string.Format("{0}, {1}", position, Forward);
	};
}
