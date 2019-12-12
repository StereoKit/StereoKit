using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace StereoKit
{
    [StructLayout(LayoutKind.Sequential)]
    public struct HandJoint
    {
        public Vec3 position;
        public Quat orientation;
        public float size;

        public Pose Pose => new Pose(position, orientation);
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Hand {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 25)]
        private HandJoint[] fingers;
        public  Pose        wrist;
        public  Pose        palm;
        public  Handed      handedness;
        public  BtnState    trackedState;
        public  BtnState    pinchState;
        public  BtnState    gripState;


        public HandJoint this[FingerId finger, JointId joint] => fingers[(int)finger * 5 + (int)joint];
        public HandJoint this[int      finger, int     joint] => fingers[finger * 5 + joint];

        public bool IsPinched       { get { return (pinchState & BtnState.Active)       > 0; } }
        public bool IsJustPinched   { get { return (pinchState & BtnState.JustActive)   > 0; } }
        public bool IsJustUnpinched { get { return (pinchState & BtnState.JustInactive) > 0; } }

        public bool IsGripped       { get { return (gripState & BtnState.Active)       > 0; } }
        public bool IsJustGripped   { get { return (gripState & BtnState.JustActive)   > 0; } }
        public bool IsJustUngripped { get { return (gripState & BtnState.JustInactive) > 0; } }

        public bool IsTracked       { get { return (trackedState & BtnState.Active)       > 0; } }
        public bool IsJustTracked   { get { return (trackedState & BtnState.JustActive)   > 0; } }
        public bool IsJustUntracked { get { return (trackedState & BtnState.JustInactive) > 0; } }

        public Material Material { set { NativeAPI.input_hand_material(handedness, value._inst); } }
        public bool     Visible  { set { NativeAPI.input_hand_visible (handedness, value ? 1 : 0); } }
        public bool     Solid    { set { NativeAPI.input_hand_solid   (handedness, value ? 1 : 0); } }
    }

    public static class Input
    {

        struct EventListener
        {
            public InputSource source;
            public BtnState type;
            public Action<InputSource, BtnState, Pointer> callback;
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
        public static Pose Head => NativeAPI.input_head();

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
            NativeAPI.input_hand_material(hand, material._inst);
        }
        public static BtnState Key(Key key)
        {
            return NativeAPI.input_key(key);
        }

        static void Initialize()
        {
            initialized = true;
            callback    = OnEvent; // This is stored in a persistant variable to force the callback from getting garbage collected!
            NativeAPI.input_subscribe(InputSource.Any, BtnState.Any, callback);
        }
        static void OnEvent(InputSource source, BtnState evt, IntPtr pointer)
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

        public static void Subscribe  (InputSource eventSource, BtnState eventTypes, Action<InputSource, BtnState, Pointer> onEvent)
        {
            if (!initialized)
                Initialize();

            EventListener item;
            item.callback = onEvent;
            item.source   = eventSource;
            item.type     = eventTypes;
            listeners.Add(item);
        }
        public static void Unsubscribe(InputSource eventSource, BtnState eventTypes, Action<InputSource, BtnState, Pointer> onEvent)
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
        public static void FireEvent  (InputSource eventSource, BtnState eventTypes, Pointer pointer)
        {
            IntPtr arg = Marshal.AllocCoTaskMem(Marshal.SizeOf<Pointer>());
            Marshal.StructureToPtr(pointer, arg, false);
            NativeAPI.input_fire_event(eventSource, eventTypes, arg);
            Marshal.FreeCoTaskMem(arg);
        }
    }
}
