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

    [Flags]
    public enum BtnState
    {
        Up       = 0,
        Down     = 1 << 0,
        JustUp   = 1 << 1,
        JustDown = 1 << 2,
        Changed  = JustUp | JustDown,
    }
    public static class BtnStateExtensions
    {
        public static bool IsPressed(this BtnState state)
        {
            return (state & BtnState.Down) > 0;
        }
        public static bool IsJustPressed(this BtnState state)
        {
            return (state & BtnState.JustDown) > 0;
        }
        public static bool IsJustReleased(this BtnState state)
        {
            return (state & BtnState.JustUp) > 0;
        }
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

    [StructLayout(LayoutKind.Sequential)]
    public struct Mouse
    {
        public int   available;
        public Vec2  pos;
        public Vec2  posChange;
        public float scroll;
        public float scrollChange;
    };

    public enum Key
    {
        MouseLeft = 0x01, MouseRight = 0x02, MouseCenter = 0x04,
        Backspace = 0x08, Tab = 0x09,
        Return = 0x0D, Shift = 0x10,
        Ctrl = 0x11, Alt = 0x12,
        Esc = 0x1B, Space = 0x20,
        End = 0x23, Home = 0x24,
        Left = 0x25, Right = 0x27,
        Up = 0x26, Down = 0x28,
        Printscreen = 0x2A, Insert = 0x2D, Del = 0x2E,
        N0 = 0x30, N1 = 0x31, N2 = 0x32, N3 = 0x33, N4 = 0x34, N5 = 0x35, N6 = 0x36, N7 = 0x37, N8 = 0x38, N9 = 0x39,
        A = 0x41, B = 0x42, C = 0x43, D = 0x44, E = 0x45, F = 0x46, G = 0x47, H = 0x48, I = 0x49, J = 0x4A, K = 0x4B, L = 0x4C, M = 0x4D, N = 0x4E, O = 0x4F, P = 0x50, Q = 0x51, R = 0x52, S = 0x53, T = 0x54, U = 0x55, V = 0x56, W = 0x57, X = 0x58, Y = 0x59, Z = 0x5A,
        LCmd = 0x5B, RCmd = 0x5C,
        Num0 = 0x60, Num1 = 0x61, Num2 = 0x62, Num3 = 0x63, Num4 = 0x64, Num5 = 0x65, Num6 = 0x66, Num7 = 0x67, Num8 = 0x68, Num9 = 0x69,
        Multiply = 0x6A, Add = 0x6B, Subtract = 0x6D, Decimal = 0x6E, Divide = 0x6F,
        F1 = 0x70, F2 = 0x71, F3 = 0x72, F4 = 0x73, F5 = 0x74, F6 = 0x75, F7 = 0x76, F8 = 0x77, F9 = 0x78, F10 = 0x79, F11 = 0x7A, F12 = 0x7B,
        MAX = 0xFF,
    };

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