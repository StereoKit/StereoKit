using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace StereoKit
{
    [StructLayout(LayoutKind.Sequential)]
    public struct HandJoint
    {
        public Vec3  position;
        public Quat  orientation;
        public float radius;

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

        public HandJoint Get(FingerId finger, JointId joint) => fingers[(int)finger * 5 + (int)joint];
        public HandJoint Get(int      finger, int     joint) => fingers[finger * 5 + joint];

        public bool IsPinched       => (pinchState & BtnState.Active)       > 0;
        public bool IsJustPinched   => (pinchState & BtnState.JustActive)   > 0;
        public bool IsJustUnpinched => (pinchState & BtnState.JustInactive) > 0;

        public bool IsGripped       => (gripState & BtnState.Active)       > 0;
        public bool IsJustGripped   => (gripState & BtnState.JustActive)   > 0;
        public bool IsJustUngripped => (gripState & BtnState.JustInactive) > 0;

        public bool IsTracked       => (trackedState & BtnState.Active)       > 0;
        public bool IsJustTracked   => (trackedState & BtnState.JustActive)   > 0;
        public bool IsJustUntracked => (trackedState & BtnState.JustInactive) > 0;

        public Material Material { set { NativeAPI.input_hand_material(handedness, value._inst); } }
        public bool     Visible  { set { NativeAPI.input_hand_visible (handedness, value); } }
        public bool     Solid    { set { NativeAPI.input_hand_solid   (handedness, value); } }
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

        public static Pose  Head  => Marshal.PtrToStructure<Pose>(NativeAPI.input_head());
        public static Mouse Mouse => Marshal.PtrToStructure<Mouse>(NativeAPI.input_mouse());

        public static int PointerCount(InputSource filter = InputSource.Any) 
            => NativeAPI.input_pointer_count(filter);
        public static Pointer Pointer(int index, InputSource filter = InputSource.Any)
            => NativeAPI.input_pointer(index, filter);
        public static Hand Hand(Handed handed)
            => Marshal.PtrToStructure<Hand>(NativeAPI.input_hand(handed));
        public static void HandVisible(Handed hand, bool visible)
            => NativeAPI.input_hand_visible(hand, visible);
        public static void HandSolid(Handed hand, bool solid)
            => NativeAPI.input_hand_solid(hand, solid);
        public static void HandMaterial(Handed hand, Material material)
            => NativeAPI.input_hand_material(hand, material._inst);
        public static BtnState Key(Key key)
            => NativeAPI.input_key(key);
        
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
