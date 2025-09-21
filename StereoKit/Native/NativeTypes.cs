using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>StereoKit initialization settings! Setup SK.settings with 
	/// your data before calling SK.Initialize.</summary>
	[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
	public struct SKSettings
	{
		private IntPtr _appName;
		private IntPtr _assetsFolder;

		/// <summary>Which operation mode should we use for this app? Default
		/// is XR, and by default the app will fall back to Simulator if XR
		/// fails or is unavailable.</summary>
		public AppMode      mode;
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
		/// one, a detailed one, one that uses stencils? By default StereoKit
		/// will let the XR runtime choose, which typically results in fast, 
		/// 16bit depth buffers for battery powered devices, and detailed 32bit
		/// depth buffers on PCs. If the requested mode is not available,
		/// StereoKit will fall back to the XR runtime's preference.</summary>
		public DepthMode    depthMode;
		/// <summary> What kind of color buffer should StereoKit use for the
		/// primary display surface? By default StereoKit will let the XR
		/// runtime choose from a list that StereoKit likes. This is generally
		/// the best choice, as the runtime can pick surface formats that can
		/// improve performance. If a requested format is not available,
		/// StereoKit will fall back to the XR runtime's preference.</summary>
		public TexFormat    colorFormat;
		/// <summary> The default log filtering level. This can be changed at
		/// runtime, but this allows you to set the log filter before
		/// Initialization occurs, so you can choose to get information from
		/// that. Default is LogLevel.Diagnostic.</summary>
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
		/// <summary>By default, StereoKit will open a desktop window for
		/// keyboard input due to lack of XR-native keyboard APIs on many
		/// platforms. If you don't want this, you can disable it with
		/// this setting!</summary>
		public  bool disableDesktopInputWindow { get { return _disableDesktopInputWindow > 0; } set { _disableDesktopInputWindow = value ? 1 : 0; } }
		private int _disableDesktopInputWindow;
		/// <summary>By default, StereoKit will slow down when the
		/// application is out of focus. This is useful for saving processing
		/// power while the app is out-of-focus, but may not always be
		/// desired. In particular, running multiple copies of a SK app for
		/// testing networking code may benefit from this setting.</summary>
		[Obsolete("Use SKSettings.standbyMode with StandbyMode.None instead.")]
		public bool disableUnfocusedSleep { get { return _disableUnfocusedSleep > 0; } set { _disableUnfocusedSleep = value ? 1 : 0; } }
		private int _disableUnfocusedSleep;

		/// <summary>If you know in advance that you need this feature, this
		/// setting allows you to set `Renderer.Scaling` before initialization.
		/// This avoids creating and discarding a large and unnecessary
		/// swapchain object. Default value is 1.</summary>
		public float renderScaling;

		/// <summary>If you know in advance that you need this feature, this
		/// setting allows you to set `Renderer.Multisample` before
		/// initialization. This avoids creating and discarding a large and
		/// unnecessary swapchain object. Default value is 1.</summary>
		public int renderMultisample;

		/// <summary>Set the behavior of StereoKit's initial origin. Default
		/// behavior is OriginMode.Local, which is the most universally
		/// supported origin mode. Different origin modes have varying levels
		/// of support on different XR runtimes, and StereoKit will provide
		/// reasonable fallbacks for each. NOTE that when falling back,
		/// StereoKit will use a different root origin mode plus an offset. You
		/// can check World.OriginMode and World.OriginOffset to inspect what
		/// StereoKit actually landed on.</summary>
		public OriginMode origin;

		/// <summary>If StereoKit has nothing to render for this frame, it
		/// skips submitting a projection layer to OpenXR entirely.</summary>
		public bool omitEmptyFrames { get { return _omitEmptyFrames > 0; } set { _omitEmptyFrames = value ? 1 : 0; } }
		private int _omitEmptyFrames;

		/// <summary>Configures StereoKit's behavior during device standby. By
		/// default in v0.4, SK will completely pause the main thread and
		/// disable audio. In v0.3, SK will continue to execute at a throttled
		/// pace, and audio will remain on.</summary>
		public StandbyMode standbyMode;

		/// <summary>A pointer to the JNI's JavaVM structure, only used for
		/// Android applications. This is optional, even for Android.</summary>
		public IntPtr androidJavaVm;
		/// <summary>A JNI reference to an android.content.Context associated
		/// with the application, only used for Android applications. Xamarin
		/// and Maui apps will use the MainActivity.Handle for this.</summary>
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

				byte[] str = NativeHelper.ToUtf8(value);
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

				byte[] str = NativeHelper.ToUtf8(value);
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

		/// <summary>Obsolete, please use Device.DisplayBlend</summary>
		[Obsolete("Obsolete, please use Device.DisplayBlend", true)]
		public DisplayBlend displayType { get => DisplayBlend.None;  set { } }
	}

	/*[StructLayout(LayoutKind.Sequential)]
	public struct FovInfo
	{
		public float left;
		public float right;
		public float top;
		public float bottom;
	}*/

	/// <summary>Visual properties and spacing of the UI system.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct UISettings
	{
		/// <summary>The margin is the space between a window and its contents.
		/// In meters.</summary>
		public float margin;
		/// <summary>Spacing between an item and its parent, in meters.</summary>
		public float padding;
		/// <summary>Spacing between sibling items, in meters.</summary>
		public float gutter;
		/// <summary>The Z depth of 3D UI elements, in meters.</summary>
		public float depth;
		/// <summary>Radius of the UI element corners, in meters.</summary>
		public float rounding;
		/// <summary>How far up does the white back-border go on UI elements?
		/// This is a 0-1 percentage of the depth value.</summary>
		public float backplateDepth;
		/// <summary>How wide is the back-border around the UI elements? In
		/// meters.</summary>
		public float backplateBorder;
		/// <summary>
		/// Defines the scale factor for the separator's thickness.
		/// The thickness is calculated by multiplying the height of the text
		/// by this factor. The default valus is 0.4f.
		/// </summary>
		public float separatorScale;
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

	/// <summary>A pretty straightforward 2D rectangle, defined by the top left
	/// corner of the rectangle, and its width/height.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct Rect
	{
		/// <summary>The X axis position of the top left corner of the
		/// rectangle.</summary>
		public float x;
		/// <summary>The Y axis position of the top left corner of the
		/// rectangle.</summary>
		public float y;
		/// <summary>The width of the rectangle.</summary>
		public float width;
		/// <summary>The height of the rectangle.</summary>
		public float height;
		/// <summary>Create a 2D rectangle, defined by the top left  corner of
		/// the rectangle, and its width/height.</summary>
		/// <param name="x">The X axis position of the top left corner of the
		/// rectangle.</param>
		/// <param name="y">The Y axis position of the top left corner of the
		/// rectangle.</param>
		/// <param name="width">The width of the rectangle.</param>
		/// <param name="height">The height of the rectangle.</param>
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

		/// <summary>This creates and fills out a LinePoint.</summary>
		/// <param name="point">The location of this point on a line.</param>
		/// <param name="color">The Color for this line vertex.</param>
		/// <param name="thickness">The thickness of the line at this vertex.
		/// </param>
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

		/// <summary>Creates a Button State using the current and previous
		/// frame's state! These two states allow us to add the "JustActive"
		/// and "JustInactive" bitflags when changes happen.</summary>
		/// <param name="wasActive">Was it active previously?</param>
		/// <param name="isActive">And is it active currently?</param>
		/// <returns>A bitflag with "Just" events added in!</returns>
		public static BtnState Make(bool wasActive, bool isActive)
		{
			BtnState result = isActive ? BtnState.Active : BtnState.Inactive;
			if (wasActive && !isActive)
				result |= BtnState.JustInactive;
			if (isActive && !wasActive)
				result |= BtnState.JustActive;
			return result;
		}
	}

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
		
		/// <summary>Ray representing the position and orientation that the
		/// current Input.Mouse.pos is pointing in.</summary>
		public Ray Ray
		{
			get
			{ unsafe {
				Ray ray;
				NativeAPI.ray_from_mouse(pos, &ray);
				return ray;
			} }
		}
	}

	/// <summary>A callback for when log events occur.</summary>
	/// <param name="level">The level of severity of this log event.</param>
	/// <param name="text">The text contents of the log event.</param>
	public delegate void LogCallback(LogLevel level, string text);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	internal unsafe delegate void LogCallbackData(IntPtr context, LogLevel level, byte* text);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	internal delegate void XRPreSessionCreateCallback(IntPtr context);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	internal delegate void XRPollEventCallback(IntPtr context, IntPtr XrEventDataBuffer);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	internal delegate void AssetOnLoadCallback(IntPtr asset, IntPtr context);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	internal delegate void RenderOnScreenshotCallback(IntPtr data, int width, int height, IntPtr context);

	/// <summary>A callback for receiving the color data of a screenshot, instead
	/// of saving it directly to a file.</summary>
	/// <param name="data">The pointer to the color data. A fare warning that the
	/// memory *will* be freed once this callback completes, so if you need to
	/// reference this data elsewhere, be sure to store a copy of it!</param>
	/// <param name="width">The width of the image.</param>
	/// <param name="height">The height of the image.</param>
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void ScreenshotCallback(IntPtr data, int width, int height);

	/// <summary>A callback that generates a sound wave at a particular point
	/// in time.</summary>
	/// <param name="time">The time along the wavelength.</param>
	/// <returns>The waveform position on the sound at this time.</returns>
	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate float AudioGenerator(float time);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	internal unsafe delegate void PickerCallback(IntPtr callback_data, int confirmed, byte* filename);

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

	/// <summary>A point on a lathe for a mesh generation algorithm. This is the
	/// 'silhouette' of the mesh, or the shape the mesh would take if you spun
	/// this line of points in a cylinder.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct UILathePt
	{
		/// <summary>Lathe point 'location', where 'x' is a percentage of the
		/// lathe radius alnong the current surface normal, and Y is the
		/// absolute Z axis value.</summary>
		public Vec2 pt;
		/// <summary>The lathe normal point, which will be rotated along the
		/// surface of the mesh.</summary>
		public Vec2 normal;
		/// <summary>Vertex color of the current lathe vertex.</summary>
		public Color32 color;
		/// <summary>Will there be triangles connecting this lathe point to the
		/// next in the list, or is this a jump without triangles?</summary>
		[MarshalAs(UnmanagedType.Bool)] public bool connectNext;
		/// <summary>Should the triangles attaching this point to the next be
		/// ordered backwards?</summary>
		[MarshalAs(UnmanagedType.Bool)] public bool flipFace;
	}

	/// <summary>Id of a simulated hand pose, for use with
	/// `Input.HandSimPoseRemove`</summary>
	public struct HandSimId
	{
		private int id;
		/// <summary>An id for testing emptiness.</summary>
		public static HandSimId None { get { return new HandSimId { id = 0 }; } }
		
		/// <summary>Compares the equality of two HandSimIds, nothing fancy
		/// here.</summary>
		/// <param name="a">First hand id.</param>
		/// <param name="b">Second hand id.</param>
		/// <returns>a == b</returns>
		public static bool operator == (HandSimId a, HandSimId b) { return a.id == b.id; }
		/// <summary>Compares the equality of two HandSimIds, nothing fancy
		/// here.</summary>
		/// <param name="a">First hand id.</param>
		/// <param name="b">Second hand id.</param>
		/// <returns>a != b</returns>
		public static bool operator != (HandSimId a, HandSimId b) { return a.id != b.id; }
		/// <summary>Same as ==</summary>
		/// <param name="obj">Must be a HandSimId</param>
		/// <returns>Equality.</returns>
		public override bool Equals(object obj)
			=> id.Equals(((HandSimId)obj).id);
		/// <summary>Hash code of the id.</summary>
		/// <returns>Hash code of the id.</returns>
		public override int GetHashCode()
			=> id.GetHashCode();
	}

	/// <summary>This represents an identifier for some item, calculated by
	/// hashing some or all of that item's data! StereoKit frequently uses id
	/// hashes to represent UI elements. See `UI.StackHash` for creating a UI
	/// identifier.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct IdHash : IEquatable<IdHash>
	{
		ulong hash;

		/// <summary>For back compatibility, allows conversion from an IdHash
		/// into a ulong, which may still be used in some parts of the older
		/// API.</summary>
		/// <param name="h">Source id.</param>
		/// <returns>An older style ulong hash.</returns>
		[Obsolete]
		public static implicit operator ulong(IdHash h) => h.hash;
		/// <summary>For back compatibility, allows old ulong hashes to auto
		/// convert to the newer opaque IdHash representation.</summary>
		/// <param name="h">Source id.</param>
		/// <returns>A new style IdHash hash.</returns>
		[Obsolete]
		public static implicit operator IdHash(ulong h) => new IdHash{ hash = h };

		/// <summary>An empty IdHash that represents the unassigned state.</summary>
		public static IdHash None => new IdHash { hash = 0 };

		/// <summary>An equality test.</summary>
		/// <param name="b">Another hash.</param>
		/// <returns>True if equal, false otherwise.</returns>
		public override bool Equals(object b) => (b is IdHash h) ? hash == h.hash : false;
		/// <summary>An equality test.</summary>
		/// <param name="b">Another hash.</param>
		/// <returns>True if equal, false otherwise.</returns>
		public bool Equals(IdHash b) => hash == b.hash;
		/// <summary>Same as ulong.GetHashCode</summary>
		/// <returns>Same as ulong.GetHashCode</returns>
		public override int GetHashCode() => hash.GetHashCode();
		/// <summary>An equality test.</summary>
		/// <param name="a">A hashed id.</param>
		/// <param name="b">A hashed id.</param>
		/// <returns>True if equal, false otherwise.</returns>
		public static bool operator ==(IdHash a, IdHash b) => a.hash == b.hash;
		/// <summary>An inequality test.</summary>
		/// <param name="a">A hashed id.</param>
		/// <param name="b">A hashed id.</param>
		/// <returns>True if unequal, false otherwise.</returns>
		public static bool operator !=(IdHash a, IdHash b) => a.hash != b.hash;
	}

	/// <summary>Information about the current state of the UI's
	/// SliderBehavior. You can use this information to draw or react to the
	/// slider's activities.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct UISliderData
	{
		/// <summary>The center location of where the slider's interaction
		/// element is.</summary>
		public Vec2     buttonCenter;
		/// <summary>The current distance of the finger, within the pressable
		/// volume of the slider, from the bottom of the slider.</summary>
		public float    fingerOffset;
		/// <summary>This is the current frame's "focus" state for the button.
		/// </summary>
		public BtnState focusState;
		/// <summary>This is the current frame's "active" state for the button.
		/// </summary>
		public BtnState activeState;
		/// <summary>The interactor that is currently driving the activity or
		/// focus of the slider. Or -1 if there is no interaction.</summary>
		public int      interactor;
	}

	/// <summary>A bit-flag enum for describing alignment or positioning.
	/// Items can be combined using the '|' operator, like so:
	/// `Align alignment = Align.YTop | Align.XLeft;`
	/// Avoid combining multiple items of the same axis. There are also a
	/// complete list of valid bit flag combinations! These are the values
	/// without an axis listed in their names, 'TopLeft', 'BottomCenter',
	/// etc.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct TextAlign
	{
		private int value;
		private TextAlign(int v) => value = v;
		private TextAlign(TextAlign v) => value = v.value;

		/// <summary>On the x axis, this item should start on the left.</summary>
		public static TextAlign XLeft = new TextAlign(1 << 0);
		/// <summary>On the y axis, this item should start at the top.</summary>
		public static TextAlign YTop = new TextAlign(1 << 1);
		/// <summary>On the x axis, the item should be centered.</summary>
		public static TextAlign XCenter = new TextAlign(1 << 2);
		/// <summary>On the y axis, the item should be centered.</summary>
		public static TextAlign YCenter = new TextAlign(1 << 3);
		/// <summary>On the x axis, this item should start on the right.</summary>
		public static TextAlign XRight = new TextAlign(1 << 4);
		/// <summary>On the y axis, this item should start on the bottom.</summary>
		public static TextAlign YBottom = new TextAlign(1 << 5);
		/// <summary>Center on both X and Y axes. This is a combination of 
		/// XCenter and YCenter.</summary>
		public static TextAlign Center = new TextAlign(XCenter | YCenter);
		/// <summary>Start on the left of the X axis, center on the Y axis. 
		/// This is a combination of XLeft and YCenter.</summary>
		public static TextAlign CenterLeft = new TextAlign(XLeft | YCenter);
		/// <summary>Start on the right of the X axis, center on the Y axis. 
		/// This is a combination of XRight and YCenter.</summary>
		public static TextAlign CenterRight = new TextAlign(XRight | YCenter);
		/// <summary>Center on the X axis, and top on the Y axis. This is a
		/// combination of XCenter and YTop.</summary>
		public static TextAlign TopCenter = new TextAlign(XCenter | YTop);
		/// <summary>Start on the left of the X axis, and top on the Y axis.
		/// This is a combination of XLeft and YTop.</summary>
		public static TextAlign TopLeft = new TextAlign(XLeft | YTop);
		/// <summary>Start on the right of the X axis, and top on the Y axis.
		/// This is a combination of XRight and YTop.</summary>
		public static TextAlign TopRight = new TextAlign(XRight | YTop);
		/// <summary>Center on the X axis, and bottom on the Y axis. This is
		/// a combination of XCenter and YBottom.</summary>
		public static TextAlign BottomCenter = new TextAlign(XCenter | YBottom);
		/// <summary>Start on the left of the X axis, and bottom on the Y
		/// axis. This is a combination of XLeft and YBottom.</summary>
		public static TextAlign BottomLeft = new TextAlign(XLeft | YBottom);
		/// <summary>Start on the right of the X axis, and bottom on the Y
		/// axis.This is a combination of XRight and YBottom.</summary>
		public static TextAlign BottomRight = new TextAlign(XRight | YBottom);

		/// <summary>Allow Flag-like enum behavior.</summary>
		/// <param name="a">First TextAlign</param>
		/// <param name="b">Second TextAlign</param>
		/// <returns>Bitwise operation of a and b.</returns>
		public static TextAlign operator |(TextAlign a, TextAlign b) => new TextAlign(a.value | b.value);
		/// <summary>Allow Flag-like enum behavior.</summary>
		/// <param name="a">First TextAlign</param>
		/// <param name="b">Second TextAlign</param>
		/// <returns>Bitwise operation of a and b.</returns>
		public static TextAlign operator &(TextAlign a, TextAlign b) => new TextAlign(a.value & b.value);
		/// <summary>Allow Flag-like enum behavior.</summary>
		/// <param name="a">First TextAlign</param>
		/// <param name="b">Second TextAlign</param>
		/// <returns>Bitwise operation of a and b.</returns>
		public static TextAlign operator ^(TextAlign a, TextAlign b) => new TextAlign(a.value ^ b.value);
		/// <summary>Allow Flag-like enum behavior.</summary>
		/// <param name="a">First TextAlign</param>
		/// <returns>Bitwise operation of a.</returns>
		public static TextAlign operator ~(TextAlign a) => new TextAlign(~a.value);

		/// <summary>For back compatibility, allows conversion from a TextAlign
		/// into an Align while providing a good obsolescence message for it.
		/// </summary>
		/// <param name="a">Source TextAlign.</param>
		/// <returns>An equivalent Align.</returns>
		[Obsolete("Use Align instead")]
		public static implicit operator Align(TextAlign a) => (Align)a;
		/// <summary>For back compatibility, allows conversion from a TextAlign
		/// into a Pivot while providing a good obsolescence message for it.
		/// </summary>
		/// <param name="a">Source TextAlign.</param>
		/// <returns>An equivalent Pivot.</returns>
		[Obsolete("Use Pivot instead")]
		public static implicit operator Pivot(TextAlign a) => (Pivot)a;
	}
}
