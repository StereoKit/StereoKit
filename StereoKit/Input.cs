using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace StereoKit
{
    [Flags]
    public enum PointerSource
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
        Pressed     = 1 << 0,
        JustPressed = 1 << 1,
        Available   = 1 << 2,
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct Pointer
    {
        public PointerSource source;
        public PointerState  state;
        public Ray           ray;

        public bool IsAvailable   { get { return (state & PointerState.Available) > 0; } }
        public bool IsPressed     { get { return (state & PointerState.Pressed) > 0; } }
        public bool IsJustPressed { get { return (state & PointerState.JustPressed) > 0; } }
    }

    public static class Input
    {
        [DllImport(Util.DllName)]
        static extern int input_pointer_count(PointerSource filter);
        [DllImport(Util.DllName)]
        static extern Pointer input_pointer(int index, PointerSource filter);

        public static int PointerCount(PointerSource filter = PointerSource.Any)
        {
            return input_pointer_count(filter);
        }
        public static Pointer Pointer(int index, PointerSource filter = PointerSource.Any)
        {
            
            return input_pointer(index, filter);
        }
    }
}
