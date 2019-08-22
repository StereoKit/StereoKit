using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

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
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void input_hand_visible(Handed hand, int visible);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void input_hand_solid(Handed hand, int solid);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void input_hand_material(Handed hand, IntPtr material);


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

        public Material Material { set { input_hand_material(handedness, value._materialInst); } }
        public bool     Visible  { set { input_hand_visible (handedness, value ? 1 : 0); } }
        public bool     Solid    { set { input_hand_solid   (handedness, value ? 1 : 0); } }
    }

    public static class Input
    {
        #region Imports
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        delegate void EventCallback(InputSource source, InputState type, IntPtr pointer);

        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern int input_pointer_count(InputSource filter);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern Pointer input_pointer(int index, InputSource filter);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr input_hand(Handed handed);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void input_subscribe(InputSource source, InputState evt, EventCallback event_callback);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void input_unsubscribe(InputSource source, InputState evt, EventCallback event_callback);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void input_fire_event(InputSource source, InputState evt, IntPtr pointer);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void input_hand_visible(Handed hand, int visible);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void input_hand_solid(Handed hand, int solid);
        [DllImport(NativeLib.DllName, CallingConvention = CallingConvention.Cdecl)]
        static extern void input_hand_material(Handed hand, IntPtr material);
        #endregion

        struct EventListener
        {
            public InputSource source;
            public InputState type;
            public Action<InputSource, InputState, Pointer> callback;
        }
        static List<EventListener> listeners   = new List<EventListener>();
        static bool                initialized = false;
        static EventCallback       callback;

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
        public static void HandVisible(Handed hand, bool visible)
        {
            input_hand_visible(hand, visible?1:0);
        }
        public static void HandSolid(Handed hand, bool solid)
        {
            input_hand_solid(hand, solid ? 1 : 0);
        }
        public static void HandMaterial(Handed hand, Material material)
        {
            input_hand_material(hand, material._materialInst);
        }

        static void Initialize()
        {
            initialized = true;
            callback    = OnEvent; // This is stored in a persistant variable to force the callback from getting garbage collected!
            input_subscribe(InputSource.Any, InputState.Any, callback);
        }
        static void OnEvent(InputSource source, InputState evt, IntPtr pointer)
        {
            Pointer ptr = Marshal.PtrToStructure<Pointer>(pointer);
            for (int i = 0; i < listeners.Count; i++)
            {
                if ((listeners[i].source & source) > 0 &&
                    (listeners[i].type   & evt) > 0)
                {
                    listeners[i].callback(source, evt, ptr);
                }
            }
        }

        public static void Subscribe  (InputSource eventSource, InputState eventTypes, Action<InputSource, InputState, Pointer> onEvent)
        {
            if (!initialized)
                Initialize();

            EventListener item;
            item.callback = onEvent;
            item.source   = eventSource;
            item.type     = eventTypes;
            listeners.Add(item);
        }
        public static void Unsubscribe(InputSource eventSource, InputState eventTypes, Action<InputSource, InputState, Pointer> onEvent)
        {
            if (!initialized)
                Initialize();

            for (int i = 0; i < listeners.Count; i++)
            {
                if (listeners[i].callback == onEvent && listeners[i].source == eventSource && listeners[i].type == eventTypes)
                {
                    listeners.RemoveAt(i);
                    i--;
                }
            }
        }
        public static void FireEvent  (InputSource eventSource, InputState eventTypes, Pointer pointer)
        {
            IntPtr arg = Marshal.AllocCoTaskMem(Marshal.SizeOf<Pointer>());
            Marshal.StructureToPtr(pointer, arg, false);
            input_fire_event(eventSource, eventTypes, arg);
            Marshal.FreeCoTaskMem(arg);
        }
    }
}
