using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace StereoKit
{
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

        public Material Material { set { NativeAPI.input_hand_material(handedness, value._materialInst); } }
        public bool     Visible  { set { NativeAPI.input_hand_visible (handedness, value ? 1 : 0); } }
        public bool     Solid    { set { NativeAPI.input_hand_solid   (handedness, value ? 1 : 0); } }
    }

    public static class Input
    {

        struct EventListener
        {
            public InputSource source;
            public InputState type;
            public Action<InputSource, InputState, Pointer> callback;
        }
        static List<EventListener> listeners   = new List<EventListener>();
        static bool                initialized = false;
        static InputEventCallback  callback;

        public static int PointerCount(InputSource filter = InputSource.Any)
        {
            return NativeAPI.input_pointer_count(filter);
        }
        public static Pointer Pointer(int index, InputSource filter = InputSource.Any)
        {
            return NativeAPI.input_pointer(index, filter);
        }
        public static Hand Hand(Handed handed)
        {
            return Marshal.PtrToStructure<Hand>(NativeAPI.input_hand(handed));
        }
        public static Mouse Mouse()
        {
            return Marshal.PtrToStructure<Mouse>(NativeAPI.input_mouse());
        }
        public static void HandVisible(Handed hand, bool visible)
        {
            NativeAPI.input_hand_visible(hand, visible?1:0);
        }
        public static void HandSolid(Handed hand, bool solid)
        {
            NativeAPI.input_hand_solid(hand, solid ? 1 : 0);
        }
        public static void HandMaterial(Handed hand, Material material)
        {
            NativeAPI.input_hand_material(hand, material._materialInst);
        }
        public static BtnState Key(Key key)
        {
            return NativeAPI.input_key(key);
        }

        static void Initialize()
        {
            initialized = true;
            callback    = OnEvent; // This is stored in a persistant variable to force the callback from getting garbage collected!
            NativeAPI.input_subscribe(InputSource.Any, InputState.Any, callback);
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
            NativeAPI.input_fire_event(eventSource, eventTypes, arg);
            Marshal.FreeCoTaskMem(arg);
        }
    }
}
