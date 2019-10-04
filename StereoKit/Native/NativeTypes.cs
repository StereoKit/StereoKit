using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
    public enum Runtime
    {
        Flatscreen   = 0,
        MixedReality = 1
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct Settings
    {
        public int flatscreenPosX;
        public int flatscreenPosY;
        public int flatscreenWidth;
        public int flatscreenHeight;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string shaderCache;
    }

    [Flags]
    public enum TexType
    {
        ImageNomips  = 1 << 0,
        Cubemap      = 1 << 1,
        Rendertarget = 1 << 2,
        Depth        = 1 << 3,
        Mips         = 1 << 4,
        Dynamic      = 1 << 5,
        Image        = ImageNomips | Mips,
    }

    public enum TexFormat
    {
        Rgba32 = 0,
        Rgba64,
        Rgba128,
        Depthstencil,
        Depth32,
        Depth16,
    }

    public enum TexSample
    {
        Linear = 0,
        Point,
        Anisotropic
    }

    public enum TexAddress
    {
        Wrap = 0,
        Clamp,
        Mirror,
    }

    public enum AlphaMode
    {
        None = 1,
        Blend,
        Test,
    }
    public enum CullMode
    {
        Ccw = 0,
        Cw,
        None,
    }

    public enum MaterialParam
    {
        Float = 0,
        Color128,
        Vector,
        Matrix,
        Texture,
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct transform_t
    {
        Vec3 _position;
        Vec3 _scale;
        Quat _rotation;

        int _dirty;
        Matrix _transform;
    }

    [Flags]
    public enum TextAlign
    {
        XLeft   = 0,
        YTop    = 0,
        XCenter = 1 << 1,
        YCenter = 1 << 2,
        XRight  = 1 << 3,
        YBottom = 1 << 4,
    }

    public enum SolidType
    {
        Normal = 0,
        Immovable,
        Unaffected,
    }

    public enum SpriteType
    {
        Atlased = 0,
        Single
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct camera_t
    {
        float fov;
        float clip_near;
        float clip_far;
    }

    [Flags]
    public enum InputSource
    {
        Any        = 0x7FFFFFFF,
        Hand       = 1 << 0,
        HandLeft   = 1 << 1,
        HandRight  = 1 << 2,
        Gaze       = 1 << 4,
        GazeHead   = 1 << 5,
        GazeEyes   = 1 << 6,
        GazeCursor = 1 << 7,
        CanPress   = 1 << 8,
    }

    [Flags]
    public enum PointerState
    {
        None        = 0,
        Available   = 1 << 0,
    }

    [Flags]
    public enum InputState
    {
        None        = 0,
        Any         = 0x7FFFFFFF,
        Tracked     = 1 << 0,
        JustTracked = 1 << 1,
        Untracked   = 1 << 2,
        Pinch       = 1 << 3,
        JustPinch   = 1 << 4,
        Unpinch     = 1 << 5,
        Grip        = 1 << 6,
        JustGrip    = 1 << 7,
        Ungrip      = 1 << 8,
    }

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void InputEventCallback(InputSource source, InputState type, IntPtr pointer);

    [StructLayout(LayoutKind.Sequential)]
    public struct Pointer
    {
        public InputSource  source;
        public PointerState state;
        public Ray          ray;
        public Quat         orientation;
    }

    public enum Handed
    {
        Left = 0,
        Right,
        Max,
    }

    public enum LogLevel
    {
        Info = 0,
        Warning,
        Error
    };
}