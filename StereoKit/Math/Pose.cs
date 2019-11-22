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

    };
}
