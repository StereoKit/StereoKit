using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>Contains information to represents a joint on the hand.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct HandJoint
	{
		/// <summary>The center of the joint's world space location.</summary>
		public Vec3 position;
		/// <summary>The joint's world space orientation, where Forward
		/// points to the next joint down the finger, and Up will point
		/// towards the back of the hand. On the left hand, Right will point
		/// towards the thumb, and on the right hand, Right will point away
		/// from the thumb.</summary>
		public Quat orientation;
		/// <summary>The distance, in meters, to the surface of the hand from
		/// this joint.</summary>
		public float radius;

		/// <summary>Pose position is the center of the joint's world space
		/// location. Pose orientation is the world space orientation, where
		/// Forward points to the next joint down the finger. On the left
		/// hand, Right will point towards the thumb, and on the right hand,
		/// Right will point away from the thumb.</summary>
		public Pose Pose => new Pose(position, orientation);

		/// <summary>You can make a hand joint of your own here, but most
		/// likely you'd rather fetch one from `Input.Hand().Get()`!</summary>
		/// <param name="position">The center of the joint's world space 
		/// location.</param>
		/// <param name="orientation">The joint's world space orientation,
		/// where Forward points to the next joint down the finger, and Up
		/// will point towards the back of the hand. On the left hand, Right
		/// will point towards the thumb, and on the right hand, Right will
		/// point away from the thumb.</param>
		/// <param name="radius">The distance, in meters, to the surface of
		/// the hand from this joint.</param>
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
		/// <summary>This is a 2D array with 25 HandJoints. You can get the
		/// right joint by `finger*5 + joint`, but really, just use Hand.Get
		/// or Hand[] instead. See Hand.Get for more info!</summary>
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 25)]
		public HandJoint[]  fingers;
		/// <summary>Pose of the wrist. TODO: Not populated right now.
		/// </summary>
		public  Pose        wrist;
		/// <summary>The position and orientation at the center of the palm!
		/// Here, Forward is the direction the flat of the palm is facing. X+
		/// is to the outside of the right hand, and to the inside of the
		/// left hand. </summary>
		public  Pose        palm;
		/// <summary>Is this a right hand, or a left hand?</summary>
		public  Handed      handed;
		/// <summary>Is the hand being tracked by the sensors right now?
		/// </summary>
		public  BtnState    tracked;
		/// <summary>Is the hand making a pinch gesture right now? Finger and
		/// thumb together.</summary>
		public  BtnState    pinch;
		/// <summary>Is the hand making a grip gesture right now? Fingers
		/// next to the palm.</summary>
		public  BtnState    grip;
		/// <summary>This is the size of the hand, calculated by measuring 
		/// the length of the middle finger! This is calculated by adding the 
		/// distances between each joint, then adding the joint radius of the
		/// root and tip. This value is recalculated at relatively frequent 
		/// intervals, and can vary by as much as a centimeter.</summary>
		public float size;
		/// <summary>What percentage of activation is the pinch gesture right
		/// now? Where 0 is a hand in an outstretched resting position, and 1
		/// is fingers touching, within a device error tolerant threshold.
		/// </summary>
		public float pinchActivation;
		/// <summary>What percentage of activation is the grip gesture right
		/// now? Where 0 is a hand in an outstretched resting position, and 1
		/// is ring finger touching the base of the palm, within a device 
		/// error tolerant threshold.</summary>
		public float gripActivation;

		public HandJoint this[FingerId finger, JointId joint] => fingers[(int)finger * 5 + (int)joint];
		public HandJoint this[int      finger, int     joint] => fingers[finger * 5 + joint];
		
		/// <summary>Returns the joint information of the indicated hand
		/// joint! This also includes fingertips as a 'joint'. This is the
		/// same as the [] operator. Note that for thumbs, there are only 4 
		/// 'joints' in reality, so StereoKit has JointId.Root and 
		/// JointId.KnuckleMajor as the same pose, so JointId.Tip is still
		/// the tip of the thumb!</summary>
		/// <param name="finger">Which finger are we getting from here, 0 is
		/// thumb, and pinky is 4!</param>
		/// <param name="joint">Which joint on the finger are we getting? 0 
		/// is the root, which is all the way at the base of the palm, and 4
		/// is the tip, the very end of the finger.</param>
		/// <returns>Position, orientation, and radius of the finger joint.
		/// </returns>
		public HandJoint Get(FingerId finger, JointId joint) => fingers[(int)finger * 5 + (int)joint];
		/// <summary>Returns the joint information of the indicated hand
		/// joint! This also includes fingertips as a 'joint'. This is the
		/// same as the [] operator. Note that for thumbs, there are only 4 
		/// 'joints' in reality, so StereoKit has JointId.Root and 
		/// JointId.KnuckleMajor as the same pose, so JointId.Tip is still
		/// the tip of the thumb!</summary>
		/// <param name="finger">Which finger are we getting from here, 0 is
		/// thumb, and pinky is 4!</param>
		/// <param name="joint">Which joint on the finger are we getting? 0 
		/// is the root, which is all the way at the base of the palm, and 4
		/// is the tip, the very end of the finger.</param>
		/// <returns>Position, orientation, and radius of the finger joint.
		/// </returns>
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

		/// <summary>Set the Material used to render the hand! The default
		/// material uses an offset of 10 to ensure it gets drawn overtop of
		/// other elements.</summary>
		public Material Material { set { NativeAPI.input_hand_material(handed, value._inst); } }
		/// <summary>Sets whether or not StereoKit should render this hand
		/// for you. Turn this to false if you're going to render your own, 
		/// or don't need the hand itself to be visible.</summary>
		public bool     Visible  { set { NativeAPI.input_hand_visible (handed, value); } }
		/// <summary>Does StereoKit register the hand with the physics
		/// system? By default, this is true. Right now this is just a single
		/// block collider, but later will involve per-joint colliders!
		/// </summary>
		public bool     Solid    { set { NativeAPI.input_hand_solid   (handed, value); } }
	}

	/// <summary>Input from the system come from this class! Hands, eyes,
	/// heads, mice and pointers!</summary>
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

		/// <summary>If the device has eye tracking hardware and the app has
		/// permission to use it, then this is the most recently tracked eye
		/// pose. Check `Input.EyesTracked` to see if the pose is up-to date,
		/// or if it's a leftover!
		/// 
		/// You can also check `SK.System.eyeTrackingPresent` to see if the 
		/// hardware is capable of providing eye tracking.
		/// 
		/// On Flatscreen when the MR sim is still enabled, then eyes are 
		/// emulated using the cursor position when the user holds down Alt.
		/// </summary>
		public static Pose     Eyes        => Marshal.PtrToStructure<Pose>(NativeAPI.input_eyes());
		/// <summary>If eye hardware is available and app has permission, 
		/// then this is the tracking state of the eyes. Eyes may move out of
		/// bounds, hardware may fail to detect eyes, or who knows what else!
		/// 
		/// On Flatscreen when MR sim is still enabled, this will report 
		/// whether the user is simulating eye input with the Alt key.
		/// </summary>
		public static BtnState EyesTracked => NativeAPI.input_eyes_tracked();

		/// <summary>The position and orientation of the user's head! This is
		/// the center point between the user's eyes, NOT the center of the
		/// user's head. Forward points the same way the user's face is 
		/// facing.</summary>
		public static Pose  Head  => Marshal.PtrToStructure<Pose>(NativeAPI.input_head());
		/// <summary>Information about this system's mouse, or lack thereof!
		/// </summary>
		public static Mouse Mouse => Marshal.PtrToStructure<Mouse>(NativeAPI.input_mouse());

		/// <summary>Retreives all the information about the user's hand!
		/// StereoKit will always provide hand information, however sometimes
		/// that information is simulated, like in the case of a mouse, or
		/// controllers. 
		/// 
		/// Note that this is a copy of the hand information, and it's a good
		/// chunk of data, so it's a good idea to grab it once and keep it
		/// around for the frame, or at least function, rather than asking
		/// for it again and again each time you want to touch something.
		/// </summary>
		/// <param name="handed">Do you want the left or the right hand?
		/// </param>
		/// <returns>A copy of the entire set of hand data!</returns>
		public static Hand Hand(Handed handed)
			=> Marshal.PtrToStructure<Hand>(NativeAPI.input_hand(handed));
		/// <summary>This allows you to completely override the hand's pose 
		/// information! It is still treated like the user's hand, so this is
		/// great for simulating input for testing purposes. It will remain
		/// overridden until you call Input.HandClearOverride.</summary>
		/// <param name="hand">Which hand should be overridden?</param>
		/// <param name="joints">A 2D array of 25 joints that should be used
		/// as StereoKit's hand information. See `Hand.fingers` for more 
		/// information.</param>
		public static void HandOverride(Handed hand, in HandJoint[] joints)
			=> NativeAPI.input_hand_override(hand, joints);
		/// <summary>Clear out the override status from Input.HandOverride,
		/// and restore the user's control over it again.</summary>
		/// <param name="hand">Which hand are we clearing the override on?
		/// </param>
		public static void HandClearOverride(Handed hand)
			=> NativeAPI.input_hand_override(hand, IntPtr.Zero);
		/// <summary>Sets whether or not StereoKit should render the hand for
		/// you. Turn this to false if you're going to render your own, or 
		/// don't need the hand itself to be visible.</summary>
		/// <param name="hand">If Handed.Max, this will set the value for 
		/// both hands.</param>
		/// <param name="visible">True, StereoKit renders this. False, it
		/// doesn't.</param>
		public static void HandVisible(Handed hand, bool visible)
			=> NativeAPI.input_hand_visible(hand, visible);
		/// <summary>Does StereoKit register the hand with the physics
		/// system? By default, this is true. Right now this is just a single
		/// block collider, but later will involve per-joint colliders!
		/// </summary>
		/// <param name="hand">If Handed.Max, this will set the value for 
		/// both hands.</param>
		/// <param name="solid">True? Physics! False? No physics.</param>
		public static void HandSolid(Handed hand, bool solid)
			=> NativeAPI.input_hand_solid(hand, solid);
		/// <summary>Set the Material used to render the hand! The default
		/// material uses an offset of 10 to ensure it gets drawn overtop of
		/// other elements.</summary>
		/// <param name="hand">>If Handed.Max, this will set the value for 
		/// both hands.</param>
		/// <param name="material">The new Material!</param>
		public static void HandMaterial(Handed hand, Material material)
			=> NativeAPI.input_hand_material(hand, material._inst);
		/// <summary>Keyboard key state! On desktop this is super handy, but
		/// even standalone MR devices can have bluetooth keyboards, or even
		/// just holographic system keyboards!</summary>
		/// <param name="key">The key to get the state of. Any key!</param>
		/// <returns>A BtnState with a number of different bits of info about
		/// whether or not the key was pressed or released this frame.</returns>
		public static BtnState Key(Key key)
			=> NativeAPI.input_key(key);

		public static int PointerCount(InputSource filter = InputSource.Any) 
			=> NativeAPI.input_pointer_count(filter);
		public static Pointer Pointer(int index, InputSource filter = InputSource.Any)
			=> NativeAPI.input_pointer(index, filter);

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
