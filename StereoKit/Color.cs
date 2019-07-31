using System.Runtime.InteropServices;

namespace StereoKit
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Color32
    {
        public byte r, g, b, a;
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct Color
    {
        public float r, g, b, a;
    }
}
