using System;
using System.Runtime.InteropServices;
using System.Text;

namespace StereoKit
{
	/// <summary>StereoKit initialization settings! Setup SK.settings with 
	/// your data before calling SK.Initialize.</summary>
	[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
	public struct SKSettings
	{
		private IntPtr _appName;
		private IntPtr _assetsFolder;

		/// <summary>Which display type should we try to load? Default is 
		/// `DisplayMode.MixedReality`.</summary>
		public DisplayMode  displayPreference;
		/// <summary>What type of background blend mode do we prefer for this
		/// application? Are you trying to build an Opaque/Immersive/VR app,
		/// or would you like the display to be AnyTransparent, so the world 
		/// will show up behind your content, if that's an option? Note that
		/// this is a preference only, and if it's not available on this 
		/// device, the app will fall back to the runtime's preference 
		/// instead! By default, (DisplayBlend.None) this uses the runtime's
		/// preference.</summary>
		public DisplayBlend blendPreference;
		/// <summary>If the preferred display fails, should we avoid falling
		/// back to flatscreen and just crash out? Default is false.</summary>
		public  bool        noFlatscreenFallback { get { return _noFlatscreenFallback>0; } set { _noFlatscreenFallback = value?1:0; } }
		private int        _noFlatscreenFallback;
		/// <summary>What kind of depth buffer should StereoKit use? A fast
		/// one, a detailed one, one that uses stencils? By default,
		/// StereoKit uses a balanced mix depending on platform, prioritizing
		/// speed but opening up when there's headroom.</summary>
		public DepthMode    depthMode;
		/// <summary> The default log filtering level. This can be changed at
		/// runtime, but this allows you to set the log filter before
		/// Initialization occurs, so you can choose to get information from
		/// that. Default is LogLevel.Info.</summary>
		public LogLevel     logFilter;
		/// <summary>If the runtime supports it, should this application run
		/// as an overlay above existing applications? Check
		/// SK.System.overlayApp after initialization to see if the runtime
		/// could comply with this flag. This will always force StereoKit to
		/// work in a blend compositing mode.</summary>
		public  bool        overlayApp { get { return _overlayApp > 0; } set { _overlayApp = value?1:0; } }
		private int        _overlayApp;
		/// <summary>For overlay applications, this is the order in which
		/// apps should be composited together. 0 means first, bottom of the
		/// stack, and uint.MaxValue is last, on top of the stack.</summary>
		public uint         overlayPriority;
		/// <summary>If using Runtime.Flatscreen, the pixel position of the
		/// window on the screen.</summary>
		public int flatscreenPosX;
		/// <summary>If using Runtime.Flatscreen, the pixel position of the
		/// window on the screen.</summary>
		public int flatscreenPosY;
		/// <summary>If using Runtime.Flatscreen, the pixel size of the
		/// window on the screen.</summary>
		public int flatscreenWidth;
		/// <summary>If using Runtime.Flatscreen, the pixel size of the
		/// window on the screen.</summary>
		public int flatscreenHeight;
		/// <summary>By default, StereoKit will simulate Mixed Reality input
		/// so developers can test MR spaces without being in a headset. If
		/// You don't want this, you can disable it with this setting!</summary>
		public  bool disableFlatscreenMRSim { get { return _disableFlatscreenMRSim > 0; } set { _disableFlatscreenMRSim = value ? 1 : 0; } }
		private int _disableFlatscreenMRSim;
		/// <summary>By default, StereoKit will slow down when the
		/// application is out of focus. This is useful for saving processing
		/// power while the app is out-of-focus, but may not always be
		/// desired. In particular, running multiple copies of a SK app for
		/// testing networking code may benefit from this setting.</summary>
		public bool disableUnfocusedSleep { get { return _disableUnfocusedSleep > 0; } set { _disableUnfocusedSleep = value ? 1 : 0; } }
		private int _disableUnfocusedSleep;

		public IntPtr androidJavaVm;
		public IntPtr androidActivity;

		/// <summary>Name of the application, this shows up an the top of the
		/// Win32 window, and is submitted to OpenXR. OpenXR caps this at 128
		/// characters.</summary>
		public string appName {
			set {
				if (_appName != IntPtr.Zero) Marshal.FreeHGlobal(_appName);

				if (string.IsNullOrEmpty(value))
				{
					_appName = IntPtr.Zero;
					return;
				}

				byte[] str = Encoding.UTF8.GetBytes(value + '\0');
				_appName = Marshal.AllocHGlobal(str.Length);
				Marshal.Copy(str, 0, _appName, str.Length);
			}
			get => Marshal.PtrToStringAnsi(_appName);
		}
		/// <summary>Where to look for assets when loading files! Final path
		/// will look like '[assetsFolder]/[file]', so a trailing '/' is
		/// unnecessary.</summary>
		public string assetsFolder {
			set {
				if (_assetsFolder != IntPtr.Zero) Marshal.FreeHGlobal(_assetsFolder);

				if (string.IsNullOrEmpty(value))
				{
					_assetsFolder = IntPtr.Zero;
					return;
				}

				byte[] str = Encoding.UTF8.GetBytes(value + '\0');
				_assetsFolder = Marshal.AllocHGlobal(str.Length);
				Marshal.Copy(str, 0, _assetsFolder, str.Length);
			}
			get => Marshal.PtrToStringAnsi(_assetsFolder);
		}
	}

	/// <summary>Information about a system's capabilities and properties!</summary>
	[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
	public struct SystemInfo
	{
		/// <summary>The type of display this device has.</summary>
		public Display displayType;
		/// <summary>Width of the display surface, in pixels! For a stereo
		/// display, this will be the width of a single eye.</summary>
		public int displayWidth;
		/// <summary>Height of the display surface, in pixels! For a stereo
		/// display, this will be the height of a single eye.</summary>
		public int displayHeight;

		/// <summary>Does the device we're currently on have the spatial
		/// graph bridge extension? The extension is provided through the
		/// function `World.FromSpatialNode`. This allows OpenXR to talk with
		/// certain windows APIs, such as the QR code API that provides Graph
		/// Node GUIDs for the pose.</summary>
		public  bool spatialBridgePresent { get => _spatialBridgePresent > 0; }
		private int _spatialBridgePresent;

		/// <summary>Can the device work with externally provided spatial
		/// anchors, like UWP's `Windows.Perception.Spatial.SpatialAnchor`
		/// </summary>
		public bool perceptionBridgePresent { get => _perceptionBridgePresent > 0; }
		private int _perceptionBridgePresent;

		/// <summary>Does the device we're on have eye tracking support
		/// present? This is _not_ an indicator that the user has given the
		/// application permission to access this information. See
		/// `Input.Gaze` for how to use this data.</summary>
		public bool eyeTrackingPresent { get => _eyeTrackingPresent > 0; }
		private int _eyeTrackingPresent;

		/// <summary>This tells if the app was successfully started as an
		/// overlay application. If this is true, then expect this
		/// application to be composited with other content below it!</summary>
		public bool overlayApp { get => _overlayApp > 0; }
		private int _overlayApp;

		/// <summary>Does this device support world occlusion of digital
		/// objects? If this is true, then World.OcclusionEnabled can be set
		/// to true, and World.OcclusionMaterial can be modified. </summary>
		public bool worldOcclusionPresent { get => _worldOcclusionPresent > 0; }
		private int _worldOcclusionPresent;

		/// <summary>Can this device get ray intersections from the
		/// environment? If this is true, then World.RaycastEnabled can be
		/// set to true, and World.Raycast can be used.</summary>
		public bool worldRaycastPresent { get => _worldRaycastPresent > 0; }
		private int _worldRaycastPresent;
	}

	/// <summary>Visual properties and spacing of the UI system.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct UISettings
	{
		/// <summary>Spacing between an item and its parent, in meters.</summary>
		public float padding;
		/// <summary>Spacing between sibling items, in meters.</summary>
		public float gutter;
		/// <summary>The Z depth of 3D UI elements, in meters.</summary>
		public float depth;
		/// <summary>How far up does the white back-border go on UI elements?
		/// This is a 0-1 percentage of the depth value.</summary>
		public float backplateDepth;
		/// <summary>How wide is the back-border around the UI elements? In
		/// meters.</summary>
		public float backplateBorder;
	}

	/// <summary>This represents a single vertex in a Mesh, all StereoKit
	/// Meshes currently use this exact layout!
	/// 
	/// It's good to fill out all values of a Vertex explicitly, as default
	/// values for the normal (0,0,0) and color (0,0,0,0) will cause your
	/// mesh to appear completely black, or even transparent in most shaders!
	/// </summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct Vertex
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

		/// <summary>Create a new Vertex, use the overloads to take advantage
		/// of default values. Vertex color defaults to White. UV defaults to
		/// (0,0).</summary>
		/// <param name="position">Location of the Vertex, this is typically
		/// meters in Model space.</param>
		/// <param name="normal">The direction the Vertex is facing. Never
		/// leave this as zero, or your lighting may turn out black! A good
		/// default value if you _don't_ know what to put here is (0,1,0),
		/// but a Mesh composed entirely of this value will have flat
		/// lighting.</param>
		public Vertex(Vec3 position, Vec3 normal)
			: this(position, normal, Vec2.Zero, Color32.White) { }
		/// <summary>Create a new Vertex, use the overloads to take advantage
		/// of default values. Vertex color defaults to White.</summary>
		/// <param name="position">Location of the Vertex, this is typically
		/// meters in Model space.</param>
		/// <param name="normal">The direction the Vertex is facing. Never
		/// leave this as zero, or your lighting may turn out black! A good
		/// default value if you _don't_ know what to put here is (0,1,0),
		/// but a Mesh composed entirely of this value will have flat
		/// lighting.</param>
		/// <param name="textureCoordinates">What part of a texture is this
		/// Vertex anchored to? (0,0) is top left of the texture, and (1,1)
		/// is the bottom right.</param>
		public Vertex(Vec3 position, Vec3 normal, Vec2 textureCoordinates)
			: this(position, normal, textureCoordinates, Color32.White) { }
		/// <summary>Create a new Vertex, use the overloads to take advantage
		/// of default values.</summary>
		/// <param name="position">Location of the Vertex, this is typically
		/// meters in Model space.</param>
		/// <param name="normal">The direction the Vertex is facing. Never
		/// leave this as zero, or your lighting may turn out black! A good
		/// default value if you _don't_ know what to put here is (0,1,0),
		/// but a Mesh composed entirely of this value will have flat
		/// lighting.</param>
		/// <param name="textureCoordinates">What part of a texture is this
		/// Vertex anchored to? (0,0) is top left of the texture, and (1,1)
		/// is the bottom right.</param>
		/// <param name="color">The color of the Vertex, StereoKit's default
		/// shaders treat this as a multiplicative modifier for the
		/// Material's albedo/diffuse color, but different shaders sometimes
		/// treat this value differently. A good default here is white, black
		/// will cause your model to turn out completely black.</param>
		public Vertex(Vec3 position, Vec3 normal, Vec2 textureCoordinates, Color32 color)
		{
			pos  = position;
			norm = normal;
			uv   = textureCoordinates;
			col  = color;
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct Rect
	{
		public float x;
		public float y;
		public float width;
		public float height;
		public Rect(float x, float y, float width, float height)
		{
			this.x = x;
			this.y = y;
			this.width = width;
			this.height = height;
		}
	}

	/// <summary>Used to represent lines for the line drawing functions! This is just a snapshot of
	/// information about each individual point on a line.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public partial struct LinePoint
	{
		/// <summary>Location of the line point</summary>
		public Vec3    pt;
		/// <summary>Total thickness of the line, in meters.</summary>
		public float   thickness;
		/// <summary>The vertex color for the line at this position.</summary>
		public Color32 color;

		
	}

	public partial struct LinePoint
	{
		public LinePoint(Vec3 point, Color32 color, float thickness)
		{
			this.pt = point;
			this.thickness = thickness;
			this.color = color;
		}
	}

	/// <summary>A collection of extension methods for the BtnState enum that makes
	/// bit-field checks a little easier.</summary>
	public static class BtnStateExtensions
	{
		/// <summary>Is the button pressed?</summary>
		/// <returns>True if pressed, false if not.</returns>
		public static bool IsActive      (this BtnState state) => (state & BtnState.Active      ) > 0;
		/// <summary>Has the button just been pressed this frame?</summary>
		/// <returns>True if pressed, false if not.</returns>
		public static bool IsJustActive  (this BtnState state) => (state & BtnState.JustActive  ) > 0;
		/// <summary>Has the button just been released this frame?</summary>
		/// <returns>True if released, false if not.</returns>
		public static bool IsJustInactive(this BtnState state) => (state & BtnState.JustInactive) > 0;
		/// <summary>Was the button either presses or released this frame?
		/// </summary>
		/// <returns>True if the state just changed this frame, false if not.
		/// </returns>
		public static bool IsChanged(this BtnState state) => (state & BtnState.Changed) > 0;
	}

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void InputEventCallback(InputSource source, BtnState type, IntPtr pointer);

	/// <summary>Pointer is an abstraction of a number of different input 
	/// sources, and a way to surface input events!</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct Pointer
	{
		/// <summary>What input source did this pointer come from? This is
		/// a bit-flag that contains input family and capability
		/// information.</summary>
		public InputSource source;
		/// <summary>Is the pointer source being tracked right now?</summary>
		public BtnState    tracked;
		/// <summary>What is the state of the input source's 'button', if it 
		/// has one?</summary>
		public BtnState    state;
		/// <summary>A ray in the direction of the pointer.</summary>
		public Ray         ray;
		/// <summary>Orientation of the pointer! Since a Ray has no concept
		/// of 'up', this can be used to retrieve more orientation information.</summary>
		public Quat        orientation;

		/// <summary>Convenience property that turns ray.position and orientation
		/// into a Pose.</summary>
		public Pose Pose => new Pose(ray.position, orientation);
	}

	/// <summary>This stores information about the mouse! What's its state, where's
	/// it pointed, do we even have one?</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct Mouse
	{
		/// <summary>Is the mouse available to use? Most MR systems likely won't have
		/// a mouse!</summary>
		public bool available;
		/// <summary>Position of the mouse relative to the window it's in! This is the number
		/// of pixels from the top left corner of the screen.</summary>
		public Vec2 pos;
		/// <summary>How much has the mouse's position changed in the current frame? Measured 
		/// in pixels.</summary>
		public Vec2 posChange;
		/// <summary>What's the current scroll value for the mouse's scroll wheel? TODO: Units</summary>
		public float scroll;
		/// <summary>How much has the scroll wheel value changed during this frame? TODO: Units</summary>
		public float scrollChange;
	}

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void LogCallback(LogLevel level, string text);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	internal delegate void AssetOnLoadCallback(IntPtr asset, IntPtr context);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate float AudioGenerator(float time);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	internal delegate void PickerCallback(IntPtr callback_data, int confirmed, IntPtr filename, int filename_length);

	/// <summary>Index values for each finger! From 0-4, from thumb to little finger.</summary>
	public enum FingerId
	{
		/// <summary>Finger 0.</summary>
		Thumb  = 0,
		/// <summary>The primary index/pointer finger! Finger 1.</summary>
		Index  = 1,
		/// <summary>Finger 2, next to the index finger.</summary>
		Middle = 2,
		/// <summary>Finger 3! What does one do with this finger? I guess... wear
		/// rings on it?</summary>
		Ring   = 3,
		/// <summary>Finger 4, the smallest little finger! AKA, The Pinky.</summary>
		Little = 4
	}

	/// <summary>Here's where hands get crazy! Technical terms, and watch out for
	/// the thumbs!</summary>
	public enum JointId
	{
		/// <summary>Joint 0. This is at the base of the hand, right above the wrist. For 
		/// the thumb, Root and KnuckleMajor have the same value.</summary>
		Root = 0,
		/// <summary>Joint 1. These are the knuckles at the top of the palm! For 
		/// the thumb, Root and KnuckleMajor have the same value.</summary>
		KnuckleMajor = 1,
		/// <summary>Joint 2. These are the knuckles in the middle of the finger! First
		/// joints on the fingers themselves.</summary>
		KnuckleMid   = 2,
		/// <summary>Joint 3. The joints right below the fingertip!</summary>
		KnuckleMinor = 3,
		/// <summary>Joint 4. The end/tip of each finger!</summary>
		Tip          = 4
	}

	/// <summary>This describes how a UI element moves when being dragged 
	/// around by a user!</summary>
	public enum UIMove
	{
		/// <summary>The element follows the position and orientation of the 
		/// user's hand exactly.</summary>
		Exact,
		/// <summary>The element follows the position of the user's hand, but
		/// orients to face the user's head instead of just using the hand's
		/// rotation.</summary>
		FaceUser,
		/// <summary>This element follows the hand's position only, completely
		/// discarding any rotation information.</summary>
		PosOnly,
		/// <summary>Do not allow user input to change the element's pose at
		/// all! You may also be interested in UI.Push/PopSurface.</summary>
		None,
	}

	/// <summary>A description of what type of window to draw! This is a bit
	/// flag, so it can contain multiple elements.</summary>
	[Flags]
	public enum UIWin
	{
		/// <summary>A normal window has a head and a body to it. Both can be
		/// grabbed.</summary>
		Normal = Head | Body,
		/// <summary>No body, no head. Not really a flag, just set to this
		/// value. The Window will still be grab/movable. To prevent it from
		/// being grabbable, combine with the UIMove.None option, or switch
		/// to UI.Push/PopSurface.</summary>
		Empty = 0,
		/// <summary>Flag to include a head on the window.</summary>
		Head = 1 << 0,
		/// <summary>Flag to include a body on the window.</summary>
		Body = 1 << 1,
	}

	/// <summary>Used with StereoKit's UI, and determines the interaction
	/// confirmation behavior for certain elements, such as the UI.HSlider!
	/// </summary>
	public enum UIConfirm
	{
		/// <summary>The user must push a button with their finger to confirm
		/// interaction with this element. This is simpler to activate as it
		/// requires no learned gestures, but may result in more false 
		/// positives.</summary>
		Push = 0,
		/// <summary>The user must use a pinch gesture to interact with this
		/// element. This is much harder to activate by accident, but does
		/// require the user to make a precise pinch gesture. You can pretty
		/// much be sure that's what the user meant to do!</summary>
		Pinch,
		/// <summary>HSlider specific. Same as Pinch, but pulling out from the
		/// slider creates a scaled slider that lets you adjust the slider at a
		/// more granular resolution.</summary>
		VariablePinch
	}

	/// <summary>Used with StereoKit's UI to indicate a particular type of UI
	/// element visual.</summary>
	public enum UIVisual
	{
		/// <summary>Default state, no UI element at all.</summary>
		None = 0,
		/// <summary>A default root UI element. Not a particular element, but
		/// other elements may refer to this if there is nothing more specific
		/// present.</summary>
		Default,
		/// <summary>Refers to UI.Button elements.</summary>
		Button,
		/// <summary>Refers to UI.Toggle elements.</summary>
		Toggle,
		/// <summary>Refers to UI.Input elements.</summary>
		Input,
		/// <summary>Refers to UI.Handle/HandleBegin elements.</summary>
		Handle,
		/// <summary>Refers to UI.Window/WindowBegin body panel element, this
		/// element is used when a Window head is also present.</summary>
		WindowBody,
		/// <summary>Refers to UI.Window/WindowBegin body element, this element
		/// is used when a Window only has the body panel, without a head.
		/// </summary>
		WindowBodyOnly,
		/// <summary>Refers to UI.Window/WindowBegin head panel element, this
		/// element is used when a Window body is also present.</summary>
		WindowHead,
		/// <summary>Refers to UI.Window/WindowBegin head element, this element
		/// is used when a Window only has the head panel, without a body.
		/// </summary>
		WindowHeadOnly,
		/// <summary>Refers to UI.HSeparator element.</summary>
		Separator,
		/// <summary>Refers to the back line component of the UI.HSlider
		/// element.</summary>
		SliderLine,
		/// <summary>Refers to the push button component of the UI.HSlider
		/// element when using UIConfirm.Push.</summary>
		SliderPush,
		/// <summary>Refers to the pinch button component of the UI.HSlider
		/// element when using UIConfirm.Pinch.</summary>
		SliderPinch,
		Max,
	}

	public enum UIColor
	{
		Primary = 0,
		Background,
		Common,
		Complement,
		Text,
		Max,
	}

	/// <summary>This specifies a particular padding mode for certain UI
	/// elements, such as the UI.Panel! This describes where padding is applied
	/// and how it affects the layout of elements.</summary>
	public enum UIPad
	{
		/// <summary>No padding, this matches the element's layout bounds
		/// exactly!</summary>
		None,
		/// <summary>This applies padding inside the element's layout bounds,
		/// and will inflate the layout bounds to fit the extra padding.</summary>
		Inside,
		/// <summary>This will apply the padding outside of the layout bounds!
		/// This will maintain the size and position of the layout volume, but
		/// the visual padding will go outside of the volume.</summary>
		Outside
	}
}
