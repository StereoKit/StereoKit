using System.Runtime.InteropServices;

namespace StereoKit
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Ray
    {
        public Vec3 position;
        public Vec3 direction;

        public Ray(Vec3 position, Vec3 direction)
        {
            this.position = position;
            this.direction = direction;
        }
    }
}
