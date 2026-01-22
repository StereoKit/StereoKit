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

	/// <summary>This represents a single vertex in a Mesh, all StereoKit Meshes
	/// currently use this exact layout!
	/// It's good to fill out all values of a Vertex explicitly, as default
	/// values for the normal (0,0,0) and color (0,0,0,0) will cause your
	/// mesh to appear completely black, or even transparent in most shaders!</summary>
	[StructLayout(LayoutKind.Sequential)]
	public partial struct Vertex
	{
		/// <summary>Position of the vertex, in model space coordinates.</summary>
		public Vec3 pos;
		/// <summary>The normal of this vertex, or the direction the vertex is
		/// facing. Preferably normalized.</summary>
		public Vec3 norm;
		/// <summary>The texture coordinates at this vertex.</summary>
		public Vec2 uv;
		/// <summary>The color of the vertex. If you aren't using it, set it to
		/// white.</summary>
		public Color32 col;
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
		/// <summary>How much has the mouse's position changed in the current frame? Measured
		/// in pixels.</summary>
		public Vec2 posChange;
		/// <summary>What's the current scroll value for the mouse's scroll wheel?</summary>
		public float scroll;
		/// <summary>How much has the scroll wheel value changed during this frame?</summary>
		public float scrollChange;
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
