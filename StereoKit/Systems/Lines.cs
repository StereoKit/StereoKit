using System;
using System.Collections.Generic;
using System.Text;

namespace StereoKit
{
    public static class Lines
    {
        public static void Add(Vec3 start, Vec3 end, Color32 color, float thickness)
            =>NativeAPI.line_add(start, end, color, thickness);
    }
}
