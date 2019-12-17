using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace StereoKit
{
    [StructLayout(LayoutKind.Sequential)]
    public struct SHLight
    {
        public Vec3 directionTo;
        public Color color;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SphericalHarmonics
    {
        private Vec3 coefficient1;
        private Vec3 coefficient2;
        private Vec3 coefficient3;
        private Vec3 coefficient4;
        private Vec3 coefficient5;
        private Vec3 coefficient6;
        private Vec3 coefficient7;
        private Vec3 coefficient8;
        private Vec3 coefficient9;

        public Color Sample(Vec3 normal) => NativeAPI.sh_lookup(this, normal);

        public static SphericalHarmonics FromLights(SHLight[] directional_lights)
            => NativeAPI.sh_create(directional_lights, directional_lights.Length);
        
    }
}
