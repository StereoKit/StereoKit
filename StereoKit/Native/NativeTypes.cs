// This is a generated file based on stereokit.h! Please don't modify it
// directly :) Instead, modify the header file, and run the StereoKitAPIGen
// project.

using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>Information about a system's capabilities and properties!</summary>
	[StructLayout(LayoutKind.Sequential)]
	public partial struct SystemInfo
	{
		/// <summary>Width of the display surface, in pixels! For a stereo display, this will
		/// be the width of a single eye.</summary>
		public int displayWidth;
		/// <summary>Height of the display surface, in pixels! For a stereo display, this will
		/// be the height of a single eye.</summary>
		public int displayHeight;
		/// <summary>Does the device we're currently on have the spatial graph bridge
		/// extension? The extension is provided through the function
		/// `World.FromSpatialNode`. This allows OpenXR to talk with certain windows
		/// APIs, such as the QR code API that provides Graph Node GUIDs for the pose.</summary>
		[MarshalAs(UnmanagedType.Bool)] public bool spatialBridgePresent;
		/// <summary>Can the device work with externally provided spatial anchors, like UWP's
		/// `Windows.Perception.Spatial.SpatialAnchor`</summary>
		[MarshalAs(UnmanagedType.Bool)] public bool perceptionBridgePresent;
		/// <summary>Does the device we're on have eye tracking support present for input
		/// purposes? This is _not_ an indicator that the user has given the
		/// application permission to access this information. See `Input.Gaze` for how
		/// to use this data.</summary>
		[MarshalAs(UnmanagedType.Bool)] public bool eyeTrackingPresent;
		/// <summary>This tells if the app was successfully started as an overlay application.
		/// If this is true, then expect this application to be composited with other
		/// content below it!</summary>
		[MarshalAs(UnmanagedType.Bool)] public bool overlayApp;
		/// <summary>Does this device support world occlusion of digital objects? If this is
		/// true, then World.OcclusionEnabled can be set to true, and
		/// World.OcclusionMaterial can be modified.</summary>
		[MarshalAs(UnmanagedType.Bool)] public bool worldOcclusionPresent;
		/// <summary>Can this device get ray intersections from the environment? If this is
		/// true, then World.RaycastEnabled can be set to true, and World.Raycast can
		/// be used.</summary>
		[MarshalAs(UnmanagedType.Bool)] public bool worldRaycastPresent;
	}

	/// <summary>This describes the field of view of a display, in degrees.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public partial struct FovInfo
	{
		/// <summary>The left edge of the field of view, in degrees.</summary>
		public float left;
		/// <summary>The right edge of the field of view, in degrees.</summary>
		public float right;
		/// <summary>The top edge of the field of view, in degrees.</summary>
		public float top;
		/// <summary>The bottom edge of the field of view, in degrees.</summary>
		public float bottom;
	}

	/// <summary>Used to represent lines for the line drawing functions! This is
	/// just a snapshot of information about each individual point on a line.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public partial struct LinePoint
	{
		/// <summary>Location of the line point.</summary>
		public Vec3 pt;
		/// <summary>Total thickness of the line, in meters.</summary>
		public float thickness;
		/// <summary>The vertex color for the line at this position.</summary>
		public Color32 color;
	}

	/// <summary>A perceptual description of the acoustic space sounds play in - an
	/// environment rather than a literal room, so it covers halls through
	/// forests. Spatial sounds feed a shared reverb whose level stays constant
	/// with distance, so the direct-to-reverb balance naturally carries how far
	/// away a sound is. A wet of 0 disables the system entirely at zero cost,
	/// and a zeroed struct is the off state. Language bindings provide preset
	/// values for common spaces as starting points.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public partial struct AudioEnvironment
	{
		/// <summary>Reverb level, 0-1. 0 turns environmental acoustics off completely,
		/// and is the default.</summary>
		public float wet;
		/// <summary>Decay time in seconds - how long the tail takes to fall 60dB at mid
		/// frequencies. Rooms are ~0.4s, cathedrals a few seconds. Clamped to
		/// 0.05-10.</summary>
		public float decay;
		/// <summary>0-1, extra high frequency decay. Soft or leafy spaces are high,
		/// tiled rooms are low.</summary>
		public float damp;
		/// <summary>Size of the space in meters, clamped to 2-40. Drives the spacing of
		/// the echoes that build the tail. Changing this restarts the tail,
		/// where the other fields all glide smoothly.</summary>
		public float size;
		/// <summary>0-1, how quickly discrete echoes blur into a dense wash. Scattered
		/// spaces like forests are high, bare rooms lower.</summary>
		public float scatter;
		/// <summary>0-1, level of the distinct early reflections off the space's
		/// surfaces - the first bounces that glue a sound to the room. The
		/// ground bounce keeps a minimum presence; walls and ceiling scale
		/// fully with this, so outdoor spaces sit near 0.</summary>
		public float reflect;
	}

	/// <summary>Pointer is an abstraction of a number of different input sources,
	/// and a way to surface input events!</summary>
	[StructLayout(LayoutKind.Sequential)]
	public partial struct Pointer
	{
		/// <summary>What input source did this pointer come from? This is a bit-flag
		/// that contains input family and capability information.</summary>
		public InputSource source;
		/// <summary>Is the pointer source being tracked right now?</summary>
		public BtnState tracked;
		/// <summary>What is the state of the input source's 'button', if it has one?</summary>
		public BtnState state;
		/// <summary>A ray in the direction of the pointer.</summary>
		public Ray ray;
		/// <summary>Orientation of the pointer! Since a Ray has no concept of 'up',
		/// this can be used to retrieve more orientation information.</summary>
		public Quat orientation;
	}

	/// <summary>This stores information about the mouse! What's its state, where's it
	/// pointed, do we even have one?</summary>
	[StructLayout(LayoutKind.Sequential)]
	public partial struct Mouse
	{
		/// <summary>Is the mouse available to use? Most MR systems likely won't have a mouse!</summary>
		[MarshalAs(UnmanagedType.Bool)] public bool available;
		/// <summary>Position of the mouse relative to the window it's in! This is the number
		/// of pixels from the top left corner of the screen.</summary>
		public Vec2 pos;
		/// <summary>How much has the mouse moved during this frame? Measured in pixels. This
		/// is all motion since the last frame, which is not always the same as the
		/// difference between this frame's position and the last frame's! In relative
		/// mouse mode, the position doesn't move at all, and this is the only place
		/// mouse motion shows up.</summary>
		public Vec2 posChange;
		/// <summary>What's the current scroll value for the mouse's scroll wheel?</summary>
		public float scroll;
		/// <summary>How much has the scroll wheel value changed during this frame?</summary>
		public float scrollChange;
	}

	/// <summary>Per-eye view metadata for a sensor depth frame, providing the
	/// camera pose and field of view used to capture that eye's depth.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public partial struct SensorDepthView
	{
		/// <summary>The pose of this eye's depth camera in world space.</summary>
		public Pose pose;
		/// <summary>The field of view of this eye's depth camera, in degrees.</summary>
		public FovInfo fov;
	}

	/// <summary>Per-frame metadata for sensor depth. Contains timestamps, dimensions,
	/// near/far planes, and per-eye camera metadata.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public partial struct SensorDepthFrame
	{
		/// <summary>The predicted display time this frame was acquired for, in OpenXR
		/// time units (nanoseconds).</summary>
		public long displayTime;
		/// <summary>The actual capture time of the depth sensor images, in OpenXR time
		/// units (nanoseconds). Zero if the runtime does not support it.</summary>
		public long captureTime;
		/// <summary>Width of a single eye's depth image, in pixels.</summary>
		public uint width;
		/// <summary>Height of a single eye's depth image, in pixels.</summary>
		public uint height;
		/// <summary>Near clip plane of the depth projection, in meters.</summary>
		public float nearZ;
		/// <summary>Far clip plane of the depth projection, in meters.</summary>
		public float farZ;
		/// <summary>Per-eye depth camera metadata. Index 0 is left, index 1 is right.</summary>
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
		public SensorDepthView[] views;
	}

	/// <summary>A point on a lathe for a mesh generation algorithm. This is the 'silhouette'
	/// of the mesh, or the shape the mesh would take if you spun this line of points
	/// in a cylinder.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public partial struct UILathePt
	{
		/// <summary>Lathe point 'location', where 'x' is a percentage of the lathe radius
		/// along the current surface normal, and Y is the absolute Z axis value.</summary>
		public Vec2 pt;
		/// <summary>The lathe normal point, which will be rotated along the surface of the
		/// mesh.</summary>
		public Vec2 normal;
		/// <summary>Vertex color of the current lathe vertex.</summary>
		public Color32 color;
		/// <summary>Will there be triangles connecting this lathe point to the next in the
		/// list, or is this a jump without triangles?</summary>
		[MarshalAs(UnmanagedType.Bool)] public bool connectNext;
		/// <summary>Should the triangles attaching this point to the next be ordered
		/// backwards?</summary>
		[MarshalAs(UnmanagedType.Bool)] public bool flipFace;
	}

	/// <summary>Visual properties and spacing of the UI system.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public partial struct UISettings
	{
		/// <summary>The margin is the space between a window and its contents, in meters.</summary>
		public float margin;
		/// <summary>Spacing between an item and its parent, in meters.</summary>
		public float padding;
		/// <summary>Spacing between sibling items, in meters.</summary>
		public float gutter;
		/// <summary>The Z depth of 3D UI elements, in meters.</summary>
		public float depth;
		/// <summary>Radius of the UI element corners, in meters.</summary>
		public float rounding;
		/// <summary>How far up does the white back-border go on UI elements? This is a 0-1 percentage of the depth value.</summary>
		public float backplateDepth;
		/// <summary>How wide is the back-border around the UI elements, in meters.</summary>
		public float backplateBorder;
		/// <summary>Defines the scale factor for the separator's thickness. The thickness is calculated by multiplying the height of the text by this factor. The default value is 0.4.</summary>
		public float separatorScale;
	}

	/// <summary>Data about a UI slider element's current interaction state. Provided by the ui_slider_behavior function.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public partial struct UISliderData
	{
		/// <summary>The center of the slider button in window-relative coordinates.</summary>
		public Vec2 buttonCenter;
		/// <summary>How far the finger is pressing into the slider, in meters.</summary>
		public float fingerOffset;
		/// <summary>The current focus state of the slider.</summary>
		public BtnState focusState;
		/// <summary>The current active/pressed state of the slider.</summary>
		public BtnState activeState;
		/// <summary>The id of the interactor that is interacting with this slider, or -1 if none.</summary>
		public int interactor;
	}

}
