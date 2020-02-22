using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace StereoKit
{
    /// <summary>Contains information to represents a joint on the hand.</summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct HandJoint
    {
        /// <summary>The joint's world space location.</summary>
        public Vec3  position;
        /// <summary>The joint's world space orientation, where Forward points 
        /// to the next joint down the finger.</summary>
        public Quat  orientation;
        /// <summary>The distance, in meters, to the surface of the hand from
        /// this joint.</summary>
        public float radius;

        /// <summary>A convenience property that wraps position and orientation
        /// into a Pose.</summary>
        public Pose Pose => new Pose(position, orientation);

        public HandJoint(Vec3 position, Quat orientation, float radius)
        {
            this.position    = position;
            this.orientation = orientation;
            this.radius      = radius;
        }
    }

    /// <summary>Information about a hand!</summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct Hand {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 25)]
        public HandJoint[]  fingers;
        /// <summary>Pose of the wrist. TODO: Not populated right now.</summary>
        public  Pose        wrist;
        /// <summary>The position and orientation at the center of the palm! Here,
        /// Forward is the direction the palm is facing. X+ is to the outside of the 
        /// right hand, and to the inside of the left hand.</summary>
        public  Pose        palm;
        /// <summary>Is this a right hand, or a left hand?</summary>
        public  Handed      handed;
        /// <summary>Is the hand being tracked by the sensors right now?</summary>
        public  BtnState    tracked;
        /// <summary>Is the hand making a pinch gesture right now? Finger and thumb together.</summary>
        public  BtnState    pinch;
        /// <summary>Is the hand making a grip gesture right now? Fingers next to the palm.</summary>
        public  BtnState    grip;
        /// <summary>What percentage of activation is the pinch gesture right now? Where 0 is a hand in
        /// an outstretched resting position, and 0 is fingers touching, within a device error tolerant
        /// threshold.</summary>
        public float pinchActivation;
        /// <summary>What percentage of activation is the grip gesture right now? Where 0 is a hand in
        /// an outstretched resting position, and 0 is ring finger touching the base of the palm, within 
        /// a device error tolerant threshold.</summary>
        public float gripActivation;

        public HandJoint this[FingerId finger, JointId joint] => fingers[(int)finger * 5 + (int)joint];
        public HandJoint this[int      finger, int     joint] => fingers[finger * 5 + joint];
		
		public HandJoint Get(FingerId finger, JointId joint) => fingers[(int)finger * 5 + (int)joint];
        public HandJoint Get(int      finger, int     joint) => fingers[finger * 5 + joint];
		
		/// <summary>Are the fingers currently pinched?</summary>
        public bool IsPinched       => (pinch & BtnState.Active)       > 0;
        /// <summary>Have the fingers just been pinched this frame?</summary>
        public bool IsJustPinched   => (pinch & BtnState.JustActive)   > 0;
        /// <summary>Have the fingers just stopped being pinched this frame?</summary>
        public bool IsJustUnpinched => (pinch & BtnState.JustInactive) > 0;

        /// <summary>Are the fingers currently gripped?</summary>
        public bool IsGripped       => (grip & BtnState.Active)       > 0;
        /// <summary>Have the fingers just been gripped this frame?</summary>
        public bool IsJustGripped   => (grip & BtnState.JustActive)   > 0;
        /// <summary>Have the fingers just stopped being gripped this frame?</summary>
        public bool IsJustUngripped => (grip & BtnState.JustInactive) > 0;

        /// <summary>Is the hand being tracked by the sensors right now?</summary>
        public bool IsTracked       => (tracked & BtnState.Active)       > 0;
        /// <summary>Has the hand just started being tracked this frame?</summary>
        public bool IsJustTracked   => (tracked & BtnState.JustActive)   > 0;
        /// <summary>Has the hand just stopped being tracked this frame?</summary>
        public bool IsJustUntracked => (tracked & BtnState.JustInactive) > 0;

        /// <summary>Set the Material used to render the hand! The default material
        /// uses an offset of -10 for optimization and transparency order.</summary>
        public Material Material { set { NativeAPI.input_hand_material(handed, value._inst); } }
        /// <summary>Should the hand be rendered?</summary>
        public bool     Visible  { set { NativeAPI.input_hand_visible (handed, value); } }
        /// <summary>Should the hand be considered solid by the physics system?</summary>
        public bool     Solid    { set { NativeAPI.input_hand_solid   (handed, value); } }
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
        public static void HandOverride(Handed hand, in HandJoint[] joints)
            => NativeAPI.input_hand_override(hand, joints);
        public static void HandClearOverride(Handed hand)
            => NativeAPI.input_hand_override(hand, IntPtr.Zero);
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
