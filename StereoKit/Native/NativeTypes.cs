using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
    /// <summary>Specifies details about how StereoKit should start up!</summary>
    public enum Runtime
    {
        /// <summary>Creates a flat, Win32 window, and simulates some MR functionality. Great for debugging.</summary>
        Flatscreen   = 0,
        /// <summary>Creates an OpenXR instance, and drives display/input through that.</summary>
        MixedReality = 1
    }

    /// <summary>StereoKit miscellaneous initialization settings! Setup StereoKit.settings with your data before calling StereoKitApp.Initialize.</summary>
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct Settings
    {
        /// <summary>If using Runtime.Flatscreen, the pixel position of the window on the screen.</summary>
        public int flatscreenPosX;
        /// <summary>If using Runtime.Flatscreen, the pixel position of the window on the screen.</summary>
        public int flatscreenPosY;
        /// <summary>If using Runtime.Flatscreen, the pixel size of the window on the screen.</summary>
        public int flatscreenWidth;
        /// <summary>If using Runtime.Flatscreen, the pixel size of the window on the screen.</summary>
        public int flatscreenHeight;
        /// <summary>Where to look for assets when loading files! Final path will look like '[assetsFolder]/[file]', so a trailing '/' is unnecessary.</summary>
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string assetsFolder;
    }

    public enum Display
    {
        Opaque = 0,
        Additive,
        Passthrough,
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct SystemInfo
    {
        public Display displayType;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct UISettings
    {
        public float padding;
        public float gutter;
        public float depth;
        public float backplateDepth;
        public float backplateBorder;
    };

    /// <summary>Textures come in various types and flavors! These are bit-flags
    /// that tell StereoKit what type of texture we want, and how the application
    /// might use it!</summary>
    [Flags]
    public enum TexType
    {
        /// <summary>A standard color image, without any generated mip-maps.</summary>
        ImageNomips  = 1 << 0,
        /// <summary>A size sided texture that's used for things like skyboxes, environment
        /// maps, and reflection probes. It behaves like a texture array with 6 textures.</summary>
        Cubemap      = 1 << 1,
        /// <summary>This texture can be rendered to! This is great for textures that might be
        /// passed in as a target to Renderer.Blit, or other such situations.</summary>
        Rendertarget = 1 << 2,
        /// <summary>This texture contains depth data, not color data!</summary>
        Depth        = 1 << 3,
        /// <summary>This texture will generate mip-maps any time the contents change. Mip-maps
        /// are a list of textures that are each half the size of the one before them! This is 
        /// used to prevent textures from 'sparkling' or aliasing in the distance.</summary>
        Mips         = 1 << 4,
        /// <summary>This texture's data will be updated frequently from the CPU (not renders)!
        /// This ensures the graphics card stores it someplace where writes are easy to do quickly.</summary>
        Dynamic      = 1 << 5,
        /// <summary>A standard color image that also generates mip-maps automatically.</summary>
        Image = ImageNomips | Mips,
    }

    /// <summary>What type of color information will the texture contain? A good default
    /// here is Rgba32.</summary>
    public enum TexFormat
    {
        /// <summary>Red/Green/Blue/Transparency data channels, at 8 bits per-channel. This is what you'll
        /// want most of the time. Matches well with the Color32 struct!</summary>
        Rgba32 = 0,
        /// <summary>Red/Green/Blue/Transparency data channels, at 16 bits per-channel! This is not
        /// common, but you might encounter it with raw photos, or HDR images.</summary>
        Rgba64,
        /// <summary>Red/Green/Blue/Transparency data channels at 32 bits per-channel! Basically 4 floats
        /// per color, which is bonkers expensive. Don't use this unless you know -exactly- what you're doing.</summary>
        Rgba128,
        /// <summary>A depth data format, 24 bits for depth data, and 8 bits to store stencil information!
        /// Stencil data can be used for things like clipping effects, deferred rendering, or shadow effects.</summary>
        DepthStencil,
        /// <summary>32 bits of data per depth value! This is pretty detailed, and is excellent for experiences
        /// that have a very far view distance.</summary>
        Depth32,
        /// <summary>16 bits of depth is not a lot, but it can be enough if your far clipping plane is pretty close.
        /// If you're seeing lots of flickering where two objects overlap, you either need to bring your far clip
        /// in, or switch to 32/24 bit depth.</summary>
        Depth16,
    }

    /// <summary>How does the shader grab pixels from the texture? Or more specifically,
    /// how does the shader grab colors between the provided pixels? If you'd like an
    /// in-depth explanation of these topics, check out [this exploration of texture filtering](https://medium.com/@bgolus/sharper-mipmapping-using-shader-based-supersampling-ed7aadb47bec)
    /// by graphics wizard Ben Golus.</summary>
    public enum TexSample
    {
        /// <summary>Use a linear blend between adjacent pixels, this creates a smooth,
        /// blurry look when texture resolution is too low.</summary>
        Linear = 0,
        /// <summary>Choose the nearest pixel's color! This makes your texture look like
        /// pixel art if you're too close.</summary>
        Point,
        /// <summary>This helps reduce texture blurriness when a surface is viewed at an
        /// extreme angle!</summary>
        Anisotropic
    }

    /// <summary>What happens when the shader asks for a texture coordinate that's outside
    /// the texture?? Believe it or not, this happens plenty often!</summary>
    public enum TexAddress
    {
        /// <summary>Wrap the UV coordinate around to the other side of the texture! This
        /// is basically like a looping texture, and is an excellent default. If you can 
        /// see weird bits of color at the edges of your texture, this may be due to Wrap
        /// blending the color with the other side of the texture, Clamp may be better in
        /// such cases.</summary>
        Wrap = 0,
        /// <summary>Clamp the UV coordinates to the edge of the texture! This'll create
        /// color streaks that continue to forever. This is actually really great for 
        /// non-looping textures that you know will always be accessed on the 0-1 range.</summary>
        Clamp,
        /// <summary>Like Wrap, but it reflects the image each time! Who needs this? I'm not sure!!
        /// But the graphics card can do it, so now you can too!</summary>
        Mirror,
    }

    /// <summary>Also known as 'alpha' for those in the know. But there's actually more than
    /// one type of transparency in rendering! The horrors. We're keepin' it fairly simple for
    /// now, so you get three options!</summary>
    public enum Transparency
    {
        /// <summary>Not actually transparent! This is opaque! Solid! It's the default option, and
        /// it's the fastest option! Opaque objects write to the z-buffer, the occlude pixels behind
        /// them, and they can be used as input to important Mixed Reality features like Late 
        /// Stage Reprojection that'll make your view more stable!</summary>
        None = 1,
        /// <summary>This will blend with the pixels behind it. This is transparent! It doesn't write
        /// to the z-buffer, and it's slower than opaque materials.</summary>
        Blend,
        /// <summary>This is sort of transparent! It can sample a texture, and discard pixels that are
        /// below a certain threshold. It doesn't blend with colors behind it, but it's pretty fast, and
        /// can write to the z-buffer no problem!</summary>
        Clip,
    }

    /// <summary>Culling is discarding an object from the render pipeline! This enum describes how mesh
    /// faces get discarded on the graphics card.</summary>
    public enum Cull
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

    [Flags]
    public enum TextAlign
    {
        XLeft   = 1 << 0,
        YTop    = 1 << 1,
        XCenter = 1 << 2,
        YCenter = 1 << 3,
        XRight  = 1 << 4,
        YBottom = 1 << 5,
        Center  = XCenter | YCenter,
    }

    /// <summary>A text style is a font plus size/color/material parameters, and are 
    /// used to keep text looking more consistent through the application by encouraging 
    /// devs to re-use styles throughout the project. See Text.MakeStyle for making a 
    /// TextStyle object.</summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct TextStyle
    {
        internal int id;
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