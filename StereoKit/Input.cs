using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace StereoKit
{
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

    [StructLayout(LayoutKind.Sequential)]
    public struct Hand {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 25)]
        public Pose[,] fingers;
        public Pose    wrist;
        public Pose    root;
        public Handed  handedness;
        public InputState state;

        public bool IsPinched       { get { return (state & InputState.Pinch)     > 0; } }
        public bool IsJustPinched   { get { return (state & InputState.JustPinch) > 0; } }
        public bool IsJustUnpinched { get { return (state & InputState.Unpinch)   > 0; } }

        public bool IsGripped       { get { return (state & InputState.Grip)     > 0; } }
        public bool IsJustGripped   { get { return (state & InputState.JustGrip) > 0; } }
        public bool IsJustUngripped { get { return (state & InputState.Ungrip)   > 0; } }

        public bool IsTracked       { get { return (state & InputState.Tracked)     > 0; } }
        public bool IsJustTracked   { get { return (state & InputState.JustTracked) > 0; } }
        public bool IsJustUntracked { get { return (state & InputState.Untracked)   > 0; } }
    }

    public static class Input
    {
        [DllImport(Util.DllName)]
        static extern int input_pointer_count(InputSource filter);
        [DllImport(Util.DllName)]
        static extern Pointer input_pointer(int index, InputSource filter);
        [DllImport(Util.DllName)]
        static extern IntPtr input_hand(Handed handed);

        public static int PointerCount(InputSource filter = InputSource.Any)
        {
            return input_pointer_count(filter);
        }
        public static Pointer Pointer(int index, InputSource filter = InputSource.Any)
        {
            return input_pointer(index, filter);
        }
        public static Hand Hand(Handed handed)
        {
            return Marshal.PtrToStructure<Hand>(input_hand(handed));
        }
    }
}
