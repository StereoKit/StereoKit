// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2024 Nick Klingensmith
// Copyright (c) 2023-2024 Qualcomm Technologies, Inc.

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

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
	public class Hand {
		/// <summary>This is a 2D array with 25 HandJoints. You can get the
		/// right joint by `finger*5 + joint`, but really, just use Hand.Get
		/// or Hand[] instead. See Hand.Get for more info!</summary>
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 25)]
		public HandJoint[]  fingers;
		/// <summary>Pose of the wrist. This is located at the base of your
		/// hand, and has a rigid orientation that points forward towards your
		/// fingers. Its orientation is unrelated to the forearm. This pose can
		/// be useful for making a hand relative coordinate space!</summary>
		public  Pose        wrist;
		/// <summary>The position and orientation of the palm! Position is
		/// specifically defined as the middle of the middle finger's root
		/// (metacarpal) bone. For orientation, Forward is the direction the
		/// flat of the palm is facing, "Iron Man" style. X+ is to the outside
		/// of the right hand, and to the inside of the left hand. </summary>
		public Pose palm;
		/// <summary>A pose an orientation representing where the hand is
		/// pointing to. This may be provided by the OpenXR runtime, or be a
		/// fallback provided by StereoKit. Typically this starts and the index
		/// finger's primary knuckle, and points in the same direction as a
		/// line drawn from the shoulder to the knuckle.</summary>
		public Pose aim;
		/// <summary>This is an approximation of where the center of a 
		/// 'pinch' gesture occurs, and is used internally by StereoKit for
		/// some tasks, such as UI. For simulated hands, this position will
		/// give you the most stable pinch location possible. For real hands,
		/// it'll be pretty close to the stablest point you'll get. This is
		/// especially important for when the user begins and ends their 
		/// pinch action, as you'll often see a lot of extra movement in the 
		/// fingers then.</summary>
		public  Vec3        pinchPt;
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
		/// <summary>This is a filter state for when the hand is ready to
		/// interact with something at a distance. This often factors into
		/// account palm direction, as well as distance from the body, and the
		/// current pinch and tracked state.</summary>
		public BtnState     aimReady;
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

		/// <summary>A 2D array accessor for getting a specific joint from the
		/// hand! This version uses an enumeration for safe access.</summary>
		/// <param name="finger">Which finger on the hand.</param>
		/// <param name="joint">Which joint on the finger. Note that for the
		/// thumb, Root and KnuckleMajor are the same.</param>
		/// <returns>The hand joint.</returns>
		public HandJoint this[FingerId finger, JointId joint] => fingers[(int)finger * 5 + (int)joint];
		/// <summary>A 2D array accessor for getting a specific joint from the
		/// hand! This version uses integers for easy iteration. Values should
		/// be in the range of [0,5).</summary>
		/// <param name="finger">Which finger on the hand, where 0 is the
		/// thumb, and 4 is the pinky.</param>
		/// <param name="joint">Which joint on the finger, where 0 is the Root,
		/// and 4 is the tip. Note that for the thumb, 0 and 1 are the same.
		/// </param>
		/// <returns>The hand joint.</returns>
		public HandJoint this[int finger, int joint] => fingers[finger * 5 + joint];
		
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
	}

	/// <summary>This represents a physical controller input device! Tracking
	/// information, buttons, analog sticks and triggers! There's also a Menu
	/// button that's tracked separately at Input.ContollerMenu.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public class Controller
	{
		/// <summary>The grip pose of the controller. This approximately
		/// represents the center of the controller where it's gripped by the
		/// user's hand. Check `trackedPos` and `trackedRot` for the current
		/// state of the pose data.</summary>
		public Pose pose;
		/// <summary>This is the pose of the hand's palm on the controller. You
		/// can use it for rendering items where the hands are when holding a
		/// controller. This pose's Forward is towards the fingers, and Up is 
		/// toward the thumbs. On the right hand, X+ goes into the palm, and on
		/// the left hand, X+ goes out of the palm. This is used by StereoKit
		/// for placing the hand mesh! </summary>
		public Pose palm;
		/// <summary>The aim pose of a controller is where the controller
		/// 'points' from and to. This is great for pointer rays and far
		/// interactions.</summary>
		public Pose aim;
		/// <summary>This tells the current tracking state of this controller
		/// overall. If either position or rotation are trackable, then this
		/// will report tracked. Typically, positional tracking will be lost
		/// first, when the controller goes out of view, and rotational 
		/// tracking will often remain as long as the controller is still 
		/// connected. This is a good way to check if the controller is
		/// connected to the system at all.</summary>
		public BtnState tracked;
		/// <summary>This tells the current tracking state of the 
		/// controller's position information. This is often the first part
		/// of tracking data to go, so it can often be good to account for 
		/// this on occasions.</summary>
		public TrackState trackedPos;
		/// <summary>This tells the current tracking state of the
		/// controller's rotational information.</summary>
		public TrackState trackedRot;
		/// <summary>This represents the click state of the controller's
		/// analog stick or directional controller.</summary>
		public BtnState stickClick;
		/// <summary>The current state of the controller's X1 button.
		/// Depending on the specific hardware, this is the first general
		/// purpose button on the controller. For example, on an Oculus Quest
		/// Touch controller this would represent 'X' on the left controller,
		/// and 'A' on the right controller.</summary>
		public BtnState x1;
		/// <summary>The current state of the controller's X2 button.
		/// Depending on the specific hardware, this is the second general
		/// purpose button on the controller. For example, on an Oculus Quest
		/// Touch controller this would represent 'Y' on the left controller,
		/// and 'B' on the right controller.</summary>
		public BtnState x2;
		/// <summary>The trigger button at the user's index finger. These
		/// buttons typically have a wide range of activation, so this is
		/// provided as a value from 0.0 -> 1.0, where 0 is no interaction,
		/// and 1 is full interaction. If a controller has binary activation,
		/// this will jump straight from 0 to 1.</summary>
		public float trigger;
		/// <summary>The grip button typically sits under the user's middle
		/// finger. These buttons occasionally have a wide range of
		/// activation, so this is provided as a value from 0.0 -> 1.0, where
		/// 0 is no interaction, and 1 is full interaction. If a controller
		/// has binary activation, this will jump straight from 0 to 1.</summary>
		public float grip;
		/// <summary>This is the current 2-axis position of the analog stick
		/// or equivalent directional controller. This generally ranges from 
		/// -1 to +1 on each axis. This is a raw input, so dead-zones and
		/// similar issues are not accounted for here, unless modified by the
		/// OpenXR platform itself.</summary>
		public Vec2 stick;

		/// <summary>Is the controller's X1 button currently pressed? 
		/// Depending on the specific hardware, this is the first general
		/// purpose button on the controller. For example, on an Oculus Quest
		/// Touch controller this would represent 'X' on the left controller,
		/// and 'A' on the right controller.</summary>
		public bool IsX1Pressed => (x1 & BtnState.Active) > 0;
		/// <summary>Has the controller's X1 button just been pressed this
		/// frame? Depending on the specific hardware, this is the first
		/// general purpose button on the controller. For example, on an
		/// Oculus Quest Touch controller this would represent 'X' on the
		/// left controller, and 'A' on the right controller.</summary>
		public bool IsX1JustPressed => (x1 & BtnState.JustActive) > 0;
		/// <summary>Has the controller's X1 button just been released this
		/// frame? Depending on the specific hardware, this is the first
		/// general purpose button on the controller. For example, on an
		/// Oculus Quest Touch controller this would represent 'X' on the
		/// left controller, and 'A' on the right controller.</summary>
		public bool IsX1JustUnPressed => (x1 & BtnState.JustInactive) > 0;

		/// <summary>Is the controller's X2 button currently pressed? 
		/// Depending on the specific hardware, this is the second general
		/// purpose button on the controller. For example, on an Oculus Quest
		/// Touch controller this would represent 'Y' on the left controller,
		/// and 'B' on the right controller.</summary>
		public bool IsX2Pressed => (x2 & BtnState.Active) > 0;
		/// <summary>Has the controller's X2 button just been pressed this
		/// frame? Depending on the specific hardware, this is the second
		/// general purpose button on the controller. For example, on an
		/// Oculus Quest Touch controller this would represent 'Y' on the
		/// left controller, and 'B' on the right controller.</summary>
		public bool IsX2JustPressed => (x2 & BtnState.JustActive) > 0;
		/// <summary>Has the controller's X2 button just been released this
		/// frame? Depending on the specific hardware, this is the second
		/// general purpose button on the controller. For example, on an
		/// Oculus Quest Touch controller this would represent 'Y' on the
		/// left controller, and 'B' on the right controller.</summary>
		public bool IsX2JustUnPressed => (x2 & BtnState.JustInactive) > 0;

		/// <summary>Is the analog stick/directional controller button 
		/// currently being actively pressed?</summary>
		public bool IsStickClicked       => (stickClick & BtnState.Active) > 0;
		/// <summary>Has the analog stick/directional controller button 
		/// just been pressed this frame?</summary>
		public bool IsStickJustClicked   => (stickClick & BtnState.JustActive) > 0;
		/// <summary>Has the analog stick/directional controller button 
		/// just been released this frame?</summary>
		public bool IsStickJustUnclicked => (stickClick & BtnState.JustInactive) > 0;

		/// <summary>Is the controller being tracked by the sensors right now?</summary>
		public bool IsTracked       => (tracked & BtnState.Active)       > 0;
		/// <summary>Has the controller just started being tracked this frame?</summary>
		public bool IsJustTracked   => (tracked & BtnState.JustActive)   > 0;
		/// <summary>Has the controller just stopped being tracked this frame?</summary>
		public bool IsJustUntracked => (tracked & BtnState.JustInactive) > 0;
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
		static Hand[]              hands       = new Hand[2] { new Hand(), new Hand() };
		static Controller[]        controllers = new Controller[2] { new Controller(), new Controller() };

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
		/// 
		/// **Permissions**
		/// - For UWP apps, permissions for eye tracking can be found in the project's .appxmanifest file under Capabilities->Gaze Input.
		/// - For Xamarin apps, you may need to add an entry to your AndroidManifest.xml, refer to your device's documentation for specifics.
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

		/// <summary>Gets raw controller input data from the system. Note that
		/// not all buttons provided here are guaranteed to be present on the
		/// user's physical controller. Controllers are also not guaranteed to
		/// be available on the system, and are never simulated.</summary>
		/// <param name="handed">The handedness of the controller to get the
		/// state of.</param>
		/// <returns>A reference to a class that contains state information 
		/// about the indicated controller.</returns>
		public static Controller Controller(Handed handed)
		{
			Marshal.PtrToStructure(NativeAPI.input_controller(handed), controllers[(int)handed]);
			return controllers[(int)handed];
		}
		/// <summary>This is the state of the controller's menu button, this
		/// is not attached to any particular hand, so it's independent of a
		/// left or right controller.</summary>
		public static BtnState ControllerMenuButton
			=> NativeAPI.input_controller_menu();

		/// <summary>When StereoKit is rendering the input source, this allows
		/// you to override the controller Model SK uses. The Model SK uses by
		/// default may be provided from the OpenXR runtime depending on
		/// extension support, but if not, SK does have a default Model.
		/// Setting this to null will restore SK's default.</summary>
		/// <param name="handed">The hand to assign the Model to.</param>
		/// <param name="model">The Model to use to represent the controller.
		/// Null is valid, and will restore SK's default model.</param>
		public static void ControllerModelSet(Handed handed, Model model)
			=> NativeAPI.input_controller_model_set(handed, model?._inst ?? IntPtr.Zero);

		/// <summary>This retreives the Model currently in use by StereoKit to
		/// represent the controller input source. By default, this will be a
		/// Model provided by OpenXR, or SK's fallback Model. This will never
		/// be null while SK is initialized.</summary>
		/// <param name="handed">The hand of the controller Model to retreive.
		/// </param>
		/// <returns>The current controller Model. By default, his will be a
		/// Model provided by OpenXR, or SK's fallback Model. This will never
		/// be null while SK is initialized.</returns>
		public static Model ControllerModelGet(Handed handed)
		{
			IntPtr model = NativeAPI.input_controller_model_get(handed);
			return model == IntPtr.Zero ? null : new Model(model);
		}

		/// <summary>Retrieves all the information about the user's hand!
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
		public static Hand Hand(Handed handed) {
			Marshal.PtrToStructure(NativeAPI.input_hand(handed), hands[(int)handed]);
			return hands[(int)handed];
		}
		/// <summary>Retrieves all the information about the user's hand!
		/// StereoKit will always provide hand information, however sometimes
		/// that information is simulated, like in the case of a mouse, or
		/// controllers.
		/// 
		/// Note that this is a copy of the hand information, and it's a good
		/// chunk of data, so it's a good idea to grab it once and keep it
		/// around for the frame, or at least function, rather than asking
		/// for it again and again each time you want to touch something.
		/// </summary>
		/// <param name="handed">Do you want the left or the right hand? 0 is
		/// left, and 1 is right.</param>
		/// <returns>A copy of the entire set of hand data!</returns>
		public static Hand Hand(int handed){
			Marshal.PtrToStructure(NativeAPI.input_hand((Handed)handed), hands[handed]);
			return hands[handed];
		}

		/// <summary>This gets the _current_ source of the hand joints! This
		/// allows you to distinguish between fully articulated joints, and
		/// simulated hand joints that may not have the same range of mobility.
		/// Note that this may change during a session, the user may put down
		/// their controllers, automatically switching to hands, or visa versa.
		/// </summary>
		/// <param name="hand">Do  you want the left or right hand? 0 is left,
		/// and 1 is right.</param>
		/// <returns>Returns information about hand tracking data source.</returns>
		public static HandSource HandSource(Handed hand) => NativeAPI.input_hand_source(hand);

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

		/// <summary>StereoKit will use controller inputs to simulate an
		/// articulated hand. This function allows you to add new simulated
		/// poses to different controller or keyboard buttons!</summary>
		/// <param name="handJointsPalmRelative25">25 joint poses, thumb to pinky, and root
		/// to tip with two duplicate poses for the thumb root joint. These
		/// should be right handed, and relative to the palm joint.</param>
		/// <param name="button1">Controller button to activate this pose, can
		/// be None if this is a keyboard only pose.</param>
		/// <param name="andButton2">Second controller button required to
		/// activate this pose. First must also be pressed. Can be None if it's
		/// only a single button pose.</param>
		/// <param name="orHotkey1">Keyboard key to activate this pose, can be
		/// None if this is a controller only pose.</param>
		/// <param name="andHotkey2">Second keyboard key required to activate
		/// this pose. First must also be pressed. Can be None if it's only a
		/// single key pose.</param>
		/// <returns>Returns the id of the hand sim pose, so it can be removed
		/// later.</returns>
		public static HandSimId HandSimPoseAdd(Pose[] handJointsPalmRelative25, ControllerKey button1 = ControllerKey.None, ControllerKey andButton2 = ControllerKey.None, Key orHotkey1 = StereoKit.Key.None, Key andHotkey2 = StereoKit.Key.None)
			=> NativeAPI.input_hand_sim_pose_add(handJointsPalmRelative25, button1, andButton2, orHotkey1, andHotkey2);
		/// <summary>Lets you remove an existing hand pose.</summary>
		/// <param name="id">Any valid or invalid hand sim pose id.</param>
		public static void HandSimPoseRemove(HandSimId id)
			=> NativeAPI.input_hand_sim_pose_remove(id);
		/// <summary>This clears all registered hand simulation poses,
		/// including the ones that StereoKit registers by default!</summary>
		public static void HandSimPoseClear()
			=> NativeAPI.input_hand_sim_pose_clear();

		/// <summary>Sets whether or not StereoKit should render the hand for
		/// you. Turn this to false if you're going to render your own, or 
		/// don't need the hand itself to be visible.</summary>
		/// <param name="hand">If Handed.Max, this will set the value for 
		/// both hands.</param>
		/// <param name="visible">True, StereoKit renders this. False, it
		/// doesn't.</param>
		public static void HandVisible(Handed hand, bool visible)
			=> NativeAPI.input_hand_visible(hand, visible);
		/// <summary>Set the Material used to render the hand! The default
		/// material uses an offset of 10 to ensure it gets drawn overtop of
		/// other elements.</summary>
		/// <param name="hand">If Handed.Max, this will set the value for 
		/// both hands.</param>
		/// <param name="material">The new Material!</param>
		public static void HandMaterial(Handed hand, Material material)
			=> NativeAPI.input_hand_material(hand, material?._inst ?? IntPtr.Zero);
		/// <summary>Keyboard key state! On desktop this is super handy, but
		/// even standalone MR devices can have bluetooth keyboards, or even
		/// just holographic system keyboards!</summary>
		/// <param name="key">The key to get the state of. Any key!</param>
		/// <returns>A BtnState with a number of different bits of info about
		/// whether or not the key was pressed or released this frame.</returns>
		public static BtnState Key(Key key)
			=> NativeAPI.input_key(key);
		/// <summary>This will inject a key press event into StereoKit's input
		/// event queue. It will be processed at the start of the next frame,
		/// and will be indistinguishable from a physical key press. Remember
		/// to release your key as well!
		/// 
		/// This will _not_ submit text to StereoKit's text queue, and will not
		/// show up in places like UI.Input. For that, you must submit a
		/// TextInjectChar call.</summary>
		/// <param name="key">The key to press.</param>
		public static void KeyInjectPress(Key key) => NativeAPI.input_key_inject_press(key);
		/// <summary>This will inject a key release event into StereoKit's
		/// input event queue. It will be processed at the start of the next
		/// frame, and will be indistinguishable from a physical key release.
		/// This should be preceded by a key press!
		/// 
		/// This will _not_ submit text to StereoKit's text queue, and will not
		/// show up in places like UI.Input. For that, you must submit a
		/// TextInjectChar call.</summary>
		/// <param name="key">The key to release.</param>
		public static void KeyInjectRelease(Key key) => NativeAPI.input_key_inject_release(key);

		/// <summary>Returns the next text character from the list of
		/// characters that have been entered this frame! Will return '\0' if
		/// there are no more characters left in the list. These are from the
		/// system's text entry system, and so can be unicode, will repeat if
		/// their 'key' is held down, and could arrive from something like a
		/// copy/paste operation.
		/// 
		/// If you wish to reset this function to begin at the start of the
		/// read list on the next call, you can call `Input.TextReset`.</summary>
		/// <returns>The next character in this frame's list, or '\0' if none
		/// remain.</returns>
		public static char TextConsume() 
			=> (char)NativeAPI.input_text_consume();
		/// <summary>Resets the `Input.TextConsume` read list back to the
		/// start.
		/// For example, `UI.Input` will _not_ call `TextReset`, so it
		/// effectively will consume those characters, hiding them from
		/// any `TextConsume` calls following it. If you wanted to check the
		/// current frame's text, but still allow `UI.Input` to work later on
		/// in the frame, you would read everything with `TextConsume`, and
		/// then `TextReset` afterwards to reset the read list for the 
		/// following `UI.Input`.</summary>
		public static void TextReset()
			=> NativeAPI.input_text_reset();
		/// <summary>This will inject a UTF32 Unicode text character into
		/// StereoKit's text input queue. It will be available at the start of
		/// the next frame, and will be indistinguishable from normal text
		/// entry.
		/// 
		/// This will _not_ submit key press/release events to StereoKit's
		/// input queue, use KeyInjectPress/Release for that.</summary>
		/// <param name="unicodeCharUTF32">An unsigned integer representing a
		/// single UTF32 character.</param>
		public static void TextInjectChar(uint unicodeCharUTF32) => NativeAPI.input_text_inject_char(unicodeCharUTF32);
		/// <summary>This will convert a C# string into a number of UTF32
		/// Unicode text characters, and inject them into StereoKit's text
		/// input queue. It will be available at the start of the next frame,
		/// and will be indistinguishable from normal text entry.
		/// 
		/// This will _not_ submit key press/release events to StereoKit's
		/// input queue, use KeyInjectPress/Release for that.</summary>
		/// <param name="chars">A collection of characters to submit as text
		/// input.</param>
		public static void TextInjectChar(string chars) {
			byte[] bytes = Encoding.Convert(Encoding.Unicode, Encoding.UTF32, Encoding.Unicode.GetBytes(chars));
			for (int i = 0; i+4 <= bytes.Length; i += 4)
				NativeAPI.input_text_inject_char(BitConverter.ToUInt32(bytes, i));
		}
		/// <summary>This will convert a byte array string into a number of
		/// UTF32 Unicode text characters, and inject them into StereoKit's
		/// text input queue. It will be available at the start of the next
		/// frame, and will be indistinguishable from normal text entry.
		/// 
		/// This will _not_ submit key press/release events to StereoKit's
		/// input queue, use KeyInjectPress/Release for that.</summary>
		/// <param name="chars">A byte array representing a string in some
		/// encoded format.</param>
		/// <param name="charEncoding">The encoding format of the byte array.
		/// Note that an encoding of UTF32 will skip converting bytes to UTF32.
		/// </param>
		public static void TextInjectChar(byte[] chars, Encoding charEncoding)
		{
			byte[] bytes = charEncoding == Encoding.UTF32 && chars.Length % 4 == 0
				? chars
				: Encoding.Convert(charEncoding, Encoding.UTF32, chars);
			for (int i = 0; i + 4 <= bytes.Length; i += 4)
				NativeAPI.input_text_inject_char(BitConverter.ToUInt32(bytes, i));
		}

		/// <summary>The number of Pointer inputs that StereoKit is tracking
		/// that match the given filter.
		/// </summary>
		/// <param name="filter">You can filter input sources using this bit
		/// flag.</param>
		/// <returns>The number of Pointers StereoKit knows about that matches
		/// the given filter.</returns>
		public static int PointerCount(InputSource filter = InputSource.Any) 
			=> NativeAPI.input_pointer_count(filter);
		/// <summary>This gets the pointer by filter based index.</summary>
		/// <param name="index">Index of the pointer.</param>
		/// <param name="filter">Filter used to search for the Pointer.</param>
		/// <returns>The Pointer data.</returns>
		public static Pointer Pointer(int index, InputSource filter = InputSource.Any)
			=> NativeAPI.input_pointer(index, filter);

		static void Initialize()
		{
			initialized = true;
			callback    = OnEvent; // This is stored in a persistent variable to force the callback from getting garbage collected!
			NativeAPI.input_subscribe(InputSource.Any, BtnState.Any, callback);
		}
		static void OnEvent(InputSource source, BtnState evt, in Pointer pointer)
		{
			for (int i = 0; i < listeners.Count; i++)
			{
				if ((listeners[i].source & source) > 0 &&
					(listeners[i].type   & evt) > 0)
				{
					listeners[i].callback(source, evt, pointer);
				}
			}
		}

		/// <summary>You can subscribe to input events from Pointer sources
		/// here. StereoKit will call your callback and pass along a Pointer
		/// that matches the position of that pointer at the moment the event
		/// occurred. This can be more accurate than polling for input data,
		/// since polling happens specifically at frame start.</summary>
		/// <param name="eventSource">What input sources do we want to listen
		/// for. This is a bit flag.</param>
		/// <param name="eventTypes">What events do we want to listen for. This
		/// is a bit flag.</param>
		/// <param name="onEvent">The callback to call when the event occurs!
		/// </param>
		public static void Subscribe(InputSource eventSource, BtnState eventTypes, Action<InputSource, BtnState, Pointer> onEvent)
		{
			if (!initialized)
				Initialize();

			EventListener item;
			item.callback = onEvent;
			item.source   = eventSource;
			item.type     = eventTypes;
			listeners.Add(item);
		}
		/// <summary>Unsubscribes a listener from input events.</summary>
		/// <param name="eventSource">The source this listener was originally
		/// registered for.</param>
		/// <param name="eventTypes">The events this listener was originally
		/// registered for.</param>
		/// <param name="onEvent">The callback this listener originally used.
		/// </param>
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
		/// <summary>This function allows you to artifically insert an input
		/// event, simulating any device source and event type you want.
		/// </summary>
		/// <param name="eventSource">The event source to simulate, this is a
		/// bit flag.</param>
		/// <param name="eventTypes">The event type to simulate, this is a bit
		/// flag.</param>
		/// <param name="pointer">The pointer data to pass along with this
		/// simulated input event.</param>
		public static void FireEvent  (InputSource eventSource, BtnState eventTypes, Pointer pointer)
		{
			IntPtr arg = Marshal.AllocCoTaskMem(Marshal.SizeOf<Pointer>());
			Marshal.StructureToPtr(pointer, arg, false);
			NativeAPI.input_fire_event(eventSource, eventTypes, arg);
			Marshal.FreeCoTaskMem(arg);
		}
	}
}
