using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace StereoKit
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Pose
    {
        public Vec3 position;
        public Quat orientation;

        public Pose(Vec3 position, Quat orientation)
        {
            this.position    = position;
            this.orientation = orientation;
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public Matrix ToMatrix(Vec3 scale) => NativeAPI.pose_matrix(this, scale);
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public Matrix ToMatrix()           => NativeAPI.pose_matrix(this, Vec3.One);

    };
}
