using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>Specifies a type of display mode StereoKit uses, like 
	/// Mixed Reality headset display vs. a PC display, or even just 
	/// rendering to an offscreen surface, or not rendering at all!</summary>
	public enum DisplayMode
	{
		/// <summary>Creates an OpenXR instance, and drives display/input 
		/// through that.</summary>
		MixedReality = 0,
		/// <summary>Creates a flat, Win32 window, and simulates some MR 
		/// functionality. Great for debugging.</summary>
		Flatscreen = 1,
		/// <summary>Not tested yet, but this is meant to run StereoKit 
		/// without rendering to any display at all. This would allow for
		/// rendering to textures, running a server that can do MR related
		/// tasks, etc.</summary>
		None = 2,
	}

	/// <summary>This is used to determine what kind of depth buffer 
	/// StereoKit uses!</summary>
	public enum DepthMode
	{
		/// <summary>Default mode, uses 16 bit on mobile devices like 
		/// HoloLens and Quest, and 32 bit on higher powered platforms like
		/// PC. If you need a far view distance even on mobile devices,
		/// prefer D32 or Stencil instead.</summary>
		Balanced = 0,
		/// <summary>16 bit depth buffer, this is fast and recommended for 
		/// devices like the HoloLens. This is especially important for fast
		/// depth based reprojection. Far view distances will suffer here 
		/// though, so keep your clipping far plane as close as possible.
		/// </summary>
		D16,
		/// <summary>32 bit depth buffer, should look great at any distance! 
		/// If you must have the best, then this is the best. If you're
		/// interested in this one, Stencil may also be plenty for you, as 24
		/// bit depth is also pretty peachy.</summary>
		D32,
		/// <summary>24 bit depth buffer with 8 bits of stencil data. 24 bits
		/// is generally plenty for a depth buffer, so using the rest for 
		/// stencil can open up some nice options! StereoKit has limited
		/// stencil support right now though (v0.3).</summary>
		Stencil,
	}

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
		/// <summary>What type of backgroud blend mode do we prefer for this
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
		/// so developers can test MR spaces without being in a headeset. If
		/// You don't want this, you can disable it with this setting!</summary>
		public  bool disableFlatscreenMRSim { get { return _disableFlatscreenMRSim > 0; } set { _disableFlatscreenMRSim = value ? 1 : 0; } }
		private int _disableFlatscreenMRSim;

		public IntPtr androidJavaVm;
		public IntPtr androidActivity;

		/// <summary>Name of the application, this shows up an the top of the
		/// Win32 window, and is submitted to OpenXR. OpenXR caps this at 128
		/// characters.</summary>
		public string appName {
			set {
				if (_appName != IntPtr.Zero) Marshal.FreeHGlobal(_appName);

				_appName = string.IsNullOrEmpty(value)
					? IntPtr.Zero
					: Marshal.StringToHGlobalAnsi(value);
			}
			get => Marshal.PtrToStringAnsi(_appName);
		}
		/// <summary>Where to look for assets when loading files! Final path 
		/// will look like '[assetsFolder]/[file]', so a trailing '/' is 
		/// unnecessary.</summary>
		public string assetsFolder { 
			set { 
				if (_assetsFolder != IntPtr.Zero) Marshal.FreeHGlobal(_assetsFolder); 

				_assetsFolder = string.IsNullOrEmpty(value)
					? IntPtr.Zero
					: Marshal.StringToHGlobalAnsi(value);
			} 
			get => Marshal.PtrToStringAnsi(_assetsFolder);
		}
	}

	// TODO: Remove in v0.4
	/// <summary>This describes the type of display tech used on a Mixed
	/// Reality device. This will be replaced by `DisplayBlend` in v0.4.
	/// </summary>
	public enum Display
	{
		/// <summary>Default value, when using this as a search type, it will
		/// fall back to default behavior which defers to platform
		/// preference.</summary>
		None = 0,
		/// <summary>This display is opaque, with no view into the real world!
		/// This is equivalent to a VR headset, or a PC screen.</summary>
		Opaque      = 1 << 0,
		/// <summary>This display is transparent, and adds light on top of
		/// the real world. This is equivalent to a HoloLens type of device.</summary>
		Additive    = 1 << 1,
		/// <summary>This is a physically opaque display, but with a camera
		/// passthrough displaying the world behind it anyhow. This would be
		/// like a Varjo XR-1, or phone-camera based AR.</summary>
		Blend       = 1 << 2,
		[Obsolete("Use Display.Blend instead, to be removed in v0.4")]
		Passthrough = 1 << 2,
		/// <summary>This matches either transparent display type! Additive
		/// or Blend. For use when you just want to see the world behind your
		/// application.</summary>
		AnyTransparent = Additive | Blend,
	}

	/// <summary>This describes the way the display's content blends with
	/// whatever is behind it. VR headsets are normally Opaque, but some VR
	/// headsets provide passthrough video, and can support Opaque as well as
	/// Blend, like the Varjo. Transparent AR displays like the HoloLens
	/// would be Additive.</summary>
	public enum DisplayBlend
	{
		/// <summary>Default value, when using this as a search type, it will
		/// fall back to default behavior which defers to platform
		/// preference.</summary>
		None = 0,
		/// <summary>This display is opaque, with no view into the real world!
		/// This is equivalent to a VR headset, or a PC screen.</summary>
		Opaque      = 1 << 0,
		/// <summary>This display is transparent, and adds light on top of
		/// the real world. This is equivalent to a HoloLens type of device.</summary>
		Additive    = 1 << 1,
		/// <summary>This is a physically opaque display, but with a camera
		/// passthrough displaying the world behind it anyhow. This would be
		/// like a Varjo XR-1, or phone-camera based AR.</summary>
		Blend       = 1 << 2,
		/// <summary>This matches either transparent display type! Additive
		/// or Blend. For use when you just want to see the world behind your
		/// application.</summary>
		AnyTransparent = Additive | Blend,
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

	/// <summary>Textures come in various types and flavors! These are bit-flags
	/// that tell StereoKit what type of texture we want, and how the application
	/// might use it!</summary>
	[Flags]
	public enum TexType
	{
		/// <summary>A standard color image, without any generated mip-maps.</summary>
		ImageNomips  = 1 << 0,
		/// <summary>A size sided texture that's used for things like skyboxes,
		/// environment maps, and reflection probes. It behaves like a texture
		/// array with 6 textures.</summary>
		Cubemap      = 1 << 1,
		/// <summary>This texture can be rendered to! This is great for textures
		/// that might be passed in as a target to Renderer.Blit, or other
		/// such situations.</summary>
		Rendertarget = 1 << 2,
		/// <summary>This texture contains depth data, not color data!</summary>
		Depth        = 1 << 3,
		/// <summary>This texture will generate mip-maps any time the contents
		/// change. Mip-maps are a list of textures that are each half the
		/// size of the one before them! This is used to prevent textures from
		/// 'sparkling' or aliasing in the distance.</summary>
		Mips         = 1 << 4,
		/// <summary>This texture's data will be updated frequently from the
		/// CPU (not renders)! This ensures the graphics card stores it
		/// someplace where writes are easy to do quickly.</summary>
		Dynamic      = 1 << 5,
		/// <summary>A standard color image that also generates mip-maps
		/// automatically.</summary>
		Image = ImageNomips | Mips,
	}

	/// <summary>What type of color information will the texture contain? A
	/// good default here is Rgba32.</summary>
	public enum TexFormat
	{
		/// <summary>A default zero value for TexFormat! Unitialized formats
		/// will get this value and **** **** up so you know to assign it
		/// properly :)</summary>
		None = 0,
		/// <summary>Red/Green/Blue/Transparency data channels, at 8 bits
		/// per-channel in sRGB color space. This is what you'll want most of
		/// the time you're dealing with color images! Matches well with the 
		/// Color32 struct! If you're storing normals, rough/metal, or
		/// anything else, use Rgba32Linear.</summary>
		Rgba32,
		/// <summary>Red/Green/Blue/Transparency data channels, at 8 bits
		/// per-channel in linear color space. This is what you'll want most
		/// of the time you're dealing with color data! Matches well with the 
		/// Color32 struct.</summary>
		Rgba32Linear,
		Bgra32,
		Bgra32Linear,
		Rg11b10,
		Rgb10a2,
		/// <summary>Red/Green/Blue/Transparency data channels, at 16 bits
		/// per-channel! This is not common, but you might encounter it with
		/// raw photos, or HDR images.</summary>
		Rgba64,
		Rgba64s,
		Rgba64f,
		/// <summary>Red/Green/Blue/Transparency data channels at 32 bits
		/// per-channel! Basically 4 floats per color, which is bonkers
		/// expensive. Don't use this unless you know -exactly- what you're
		/// doing.</summary>
		Rgba128,
		/// <summary>A single channel of data, with 8 bits per-pixel! This
		/// can be great when you're only using one channel, and want to 
		/// reduce memory usage. Values in the shader are always 0.0-1.0.
		/// </summary>
		R8,
		/// <summary>A single channel of data, with 16 bits per-pixel! This
		/// is a good format for height maps, since it stores a fair bit of
		/// information in it. Values in the shader are always 0.0-1.0.
		/// </summary>
		R16,
		/// <summary>A single channel of data, with 32 bits per-pixel! This 
		/// basically treats each pixel as a generic float, so you can do all
		/// sorts of strange and interesting things with this.</summary>
		R32,
		/// <summary>A depth data format, 24 bits for depth data, and 8 bits
		/// to store stencil information! Stencil data can be used for things
		/// like clipping effects, deferred rendering, or shadow effects.</summary>
		DepthStencil,
		/// <summary>32 bits of data per depth value! This is pretty detailed, 
		/// and is excellent for experiences that have a very far view distance.</summary>
		Depth32,
		/// <summary>16 bits of depth is not a lot, but it can be enough if
		/// your far clipping plane is pretty close. If you're seeing lots of
		/// flickering where two objects overlap, you either need to bring
		/// your far clip in, or switch to 32/24 bit depth.</summary>
		Depth16,
	}

	/// <summary>How does the shader grab pixels from the texture? Or more
	/// specifically, how does the shader grab colors between the provided 
	/// pixels? If you'd like an in-depth explanation of these topics, check
	/// out [this exploration of texture filtering](https://medium.com/@bgolus/sharper-mipmapping-using-shader-based-supersampling-ed7aadb47bec)
	/// by graphics wizard Ben Golus.</summary>
	public enum TexSample
	{
		/// <summary>Use a linear blend between adjacent pixels, this creates
		/// a smooth, blurry look when texture resolution is too low.</summary>
		Linear = 0,
		/// <summary>Choose the nearest pixel's color! This makes your texture
		/// look like pixel art if you're too close.</summary>
		Point,
		/// <summary>This helps reduce texture blurriness when a surface is
		/// viewed at an extreme angle!</summary>
		Anisotropic
	}

	/// <summary>What happens when the shader asks for a texture coordinate
	/// that's outside the texture?? Believe it or not, this happens plenty
	/// often!</summary>
	public enum TexAddress
	{
		/// <summary>Wrap the UV coordinate around to the other side of the texture! This
		/// is basically like a looping texture, and is an excellent default. If you can 
		/// see weird bits of color at the edges of your texture, this may be due to Wrap
		/// blending the color with the other side of the texture, Clamp may be better in
		/// such cases.</summary>
		Wrap = 0,
		/// <summary>Clamp the UV coordinates to the edge of the texture! This'll create
		/// color streaks that continue to forever. This is actually really great for 
		/// non-looping textures that you know will always be accessed on the 0-1 range.</summary>
		Clamp,
		/// <summary>Like Wrap, but it reflects the image each time! Who needs this? I'm not sure!!
		/// But the graphics card can do it, so now you can too!</summary>
		Mirror,
	}

	/// <summary>Also known as 'alpha' for those in the know. But there's 
	/// actually more than one type of transparency in rendering! The 
	/// horrors. We're keepin' it fairly simple for now, so you get three
	/// options!</summary>
	public enum Transparency
	{
		/// <summary>Not actually transparent! This is opaque! Solid! It's
		/// the default option, and it's the fastest option! Opaque objects
		/// write to the z-buffer, the occlude pixels behind them, and they
		/// can be used as input to important Mixed Reality features like 
		/// Late Stage Reprojection that'll make your view more stable!
		/// </summary>
		None = 1,
		/// <summary>This will blend with the pixels behind it. This is 
		/// transparent! It doesn't write to the z-buffer, and it's slower
		/// than opaque materials.</summary>
		Blend,
		/// <summary>This will straight up add the pixel color to the color
		/// buffer! This usually looks -really- glowy, so it makes for good
		/// particles or lighting effects.</summary>
		Add
	}

	/// <summary>Culling is discarding an object from the render pipeline!
	/// This enum describes how mesh faces get discarded on the graphics
	/// card. With culling set to none, you can double the number of pixels 
	/// the GPU ends up drawing, which can have a big impact on performance. 
	/// None can be appropriate in cases where the mesh is designed to be 
	/// 'double sided'. Front can also be helpful when you want to flip a 
	/// mesh 'inside-out'!</summary>
	public enum Cull
	{
		/// <summary>Discard if the back of the triangle face is pointing 
		/// towards the camera. This is the default behavior.</summary>
		Back = 0,
		/// <summary>Discard if the front of the triangle face is pointing 
		/// towards the camera. This is opposite the default behavior.
		/// </summary>
		Front,
		/// <summary>No culling at all! Draw the triangle regardless of which
		/// way it's pointing.</summary>
		None,
	}

	/// <summary>Depth test describes how this material looks at and responds
	/// to depth information in the zbuffer! The default is Less, which means
	/// if the material pixel's depth is Less than the existing depth data,
	/// (basically, is this in front of some other object) it will draw that
	/// pixel. Similarly, Greater would only draw the material if it's
	/// 'behind' the depth buffer. Always would just draw all the time, and 
	/// not read from the depth buffer at all.</summary>
	public enum DepthTest
	{
		/// <summary>Default behavior, pixels behind the depth buffer will be
		/// discarded, and pixels in front of it will be drawn.</summary>
		Less = 0,
		/// <summary>Pixels behind the depth buffer will be discarded, and
		/// pixels in front of, or at the depth buffer's value it will be
		/// drawn. This could be great for things that might be sitting 
		/// exactly on a floor or wall.</summary>
		LessOrEqual,
		/// <summary>Pixels in front of the zbuffer will be discarded! This 
		/// is opposite of how things normally work. Great for drawing 
		/// indicators that something is occluded by a wall or other 
		/// geometry.</summary>
		Greater,
		/// <summary>Pixels in front of (or exactly at) the zbuffer will be 
		/// discarded! This is opposite of how things normally work. Great
		/// for drawing indicators that something is occluded by a wall or
		/// other geometry.</summary>
		GreaterOrEqual,
		/// <summary>Only draw pixels if they're at exactly the same depth as
		/// the zbuffer!</summary>
		Equal,
		/// <summary>Draw any pixel that's not exactly at the value in the
		/// zbuffer.</summary>
		NotEqual,
		/// <summary>Don't look at the zbuffer at all, just draw everything, 
		/// always, all the time! At this poit, the order at which the mesh
		/// gets drawn will be  super important, so don't forget about 
		/// `Material.QueueOffset`!</summary>
		Always,
		/// <summary>Never draw a pixel, regardless of what's in the zbuffer.
		/// I can think of better ways to do this, but uhh, this is here for
		/// completeness! Maybe you can find a use for it.</summary>
		Never,
	}

	/// <summary>What type of data does this material parameter need? This is used to tell the 
	/// shader how large the data is, and where to attach it to on the shader.</summary>
	public enum MaterialParam
	{
		/// <summary>A single 32 bit float value.</summary>
		Float = 0,
		/// <summary>A color value described by 4 floating point values.</summary>
		Color128,
		/// <summary>A 4 component vector composed of loating point values.</summary>
		Vector,
		/// <summary>A 4x4 matrix of floats.</summary>
		Matrix,
		/// <summary>Texture information!</summary>
		Texture,
	}

	/// <summary>Describes how an animation is played back, and what to do when
	/// the animation hits the end.</summary>
	public enum AnimMode
	{
		/// <summary> If the animation reaches the end, it will always loop
		/// back around to the start again.</summary>
		Loop,
		/// <summary> When the animation reaches the end, it will freeze
		/// in-place.</summary>
		Once,
		/// <summary> The animation will not progress on its own, and instead
		/// must be driven by providing information to the model's AnimTime
		/// or AnimCompletion properties.</summary>
		Manual,
	}

	/// <summary>A bit-flag enum for describing alignment or positioning. 
	/// Items can be combined using the '|' operator, like so:
	/// 
	/// `TextAlign alignment = TextAlign.YTop | TextAlign.XLeft;`
	/// 
	/// Avoid combining multiple items of the same axis. There are also a
	/// complete list of valid bit flag combinations! These are the values
	/// without an axis listed in their names, 'TopLeft', 'BottomCenter', 
	/// etc.</summary>
	[Flags]
	public enum TextAlign
	{
		/// <summary>On the x axis, this item should start on the left.</summary>
		XLeft   = 1 << 0,
		/// <summary>On the y axis, this item should start at the top.</summary>
		YTop    = 1 << 1,
		/// <summary>On the x axis, the item should be centered.</summary>
		XCenter = 1 << 2,
		/// <summary>On the y axis, the item should be centered.</summary>
		YCenter = 1 << 3,
		/// <summary>On the x axis, this item should start on the right.</summary>
		XRight  = 1 << 4,
		/// <summary>On the y axis, this item should start on the bottom.</summary>
		YBottom = 1 << 5,
		/// <summary>Center on both X and Y axes. This is a combination of 
		/// XCenter and YCenter.</summary>
		Center      = XCenter | YCenter,
		/// <summary>Start on the left of the X axis, center on the Y axis. 
		/// This is a combination of XLeft and YCenter.</summary>
		CenterLeft  = XLeft   | YCenter,
		/// <summary>Start on the right of the X axis, center on the Y axis. 
		/// This is a combination of XRight and YCenter.</summary>
		CenterRight = XRight  | YCenter,
		/// <summary>Start on the left of the X axis, and top on the Y axis.
		/// This is a combination of XLeft and YTop.</summary>
		TopLeft     = XLeft   | YTop,
		/// <summary>Start on the right of the X axis, and top on the Y axis.
		/// This is a combination of XRight and YTop.</summary>
		TopRight    = XRight  | YTop,
		/// <summary>Center on the X axis, and top on the Y axis. This is a
		/// combination of XCenter and YTop.</summary>
		TopCenter   = XCenter | YTop,
		/// <summary>Start on the left of the X axis, and bottom on the Y
		/// axis. This is a combination of XLeft and YBottom.</summary>
		BottomLeft  = XLeft   | YBottom,
		/// <summary>Start on the right of the X axis, and bottom on the Y
		/// axis.This is a combination of XRight and YBottom.</summary>
		BottomRight = XRight  | YBottom,
		/// <summary>Center on the X axis, and bottom on the Y axis. This is
		/// a combination of XCenter and YBottom.</summary>
		BottomCenter= XCenter | YBottom,
	}

	/// <summary>This enum describes how text layout behaves within the space
	/// it is given.</summary>
	[Flags]
	public enum TextFit
	{
		/// <summary>The text will wrap around to the next line down when it
		/// reaches the end of the space on the X axis.</summary>
		Wrap     = 1 << 0,
		/// <summary>When the text reaches the end, it is simply truncated
		/// and no longer visible.</summary>
		Clip     = 1 << 1,
		/// <summary>If the text is too large to fit in the space provided,
		/// it will be scaled down to fit inside. This will not scale up.
		/// </summary>
		Squeeze  = 1 << 2,
		/// <summary>If the text is larger, or smaller than the space 
		/// provided, it will scale down or up to fill the space.</summary>
		Exact    = 1 << 3,
		/// <summary>The text will ignore the containing space, and just keep
		/// on going.</summary>
		Overflow = 1 << 4
	}

	/// <summary>This describes the behavior of a 'Solid' physics object! the physics 
	/// engine will apply forces differently </summary>
	public enum SolidType
	{
		/// <summary>This object behaves like a normal physical object, it'll fall, get
		/// pushed around, and generally be succeptible to physical forces! This is a 
		/// 'Dynamic' body in physics simulation terms.</summary>
		Normal = 0,
		/// <summary>Immovable objects are always stationary! They have infinite mass,
		/// zero velocity, and can't collide with Immovable of Unaffected types.</summary>
		Immovable,
		/// <summary>Unaffected objects have infinite mass, but can have a velocity! They'll
		/// move under their own forces, but nothing in the simulation will affect them. 
		/// They don't collide with Immovable or Unaffected types.</summary>
		Unaffected,
	}

	/// <summary>The way the Sprite is stored on the backend! Does it get batched and atlased 
	/// for draw efficiency, or is it a single image?</summary>
	public enum SpriteType
	{
		/// <summary>The sprite will be batched onto an atlas texture so all sprites can be 
		/// drawn in a single pass. This is excellent for performance! The only thing to 
		/// watch out for here, adding a sprite to an atlas will rebuild the atlas texture!
		/// This can be a bit expensive, so it's recommended to add all sprites to an atlas
		/// at start, rather than during runtime. Also, if an image is too large, it may take
		/// up too much space on the atlas, and may be better as a Single sprite type.</summary>
		Atlased = 0,
		/// <summary>This sprite is on its own texture. This is best for large images, items
		/// that get loaded and unloaded during runtime, or for sprites that may have edge
		/// artifacts or severe 'bleed' from adjacent atlased images.</summary>
		Single
	}

	/// <summary>Used to represent lines for the line drawing functions! This is just a snapshot of
	/// information about each individual point on a line.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct LinePoint
	{
		/// <summary>Location of the line point</summary>
		public Vec3    pt;
		/// <summary>Total thickness of the line, in meters.</summary>
		public float   thickness;
		/// <summary>The vertex color for the line at this position.</summary>
		public Color32 color;

		public LinePoint(Vec3 point, Color32 color, float thickness)
		{
			this.pt        = point;
			this.thickness = thickness;
			this.color     = color;
		}
	}

	public enum RenderLayer
	{
		Layer0 = 1 << 0,
		Layer1 = 1 << 1,
		Layer2 = 1 << 2,
		Layer3 = 1 << 3,
		Layer4 = 1 << 4,
		Layer5 = 1 << 5,
		Layer6 = 1 << 6,
		Layer7 = 1 << 7,
		Layer8 = 1 << 8,
		Layer9 = 1 << 9,
		Vfx    = 1 << 10,
		All    = 0xFFFF,
		AllRegular = Layer0 | Layer1 | Layer2 | Layer3 | Layer4 | Layer5 | Layer6 | Layer7 | Layer8 | Layer9,
	}

	public enum RenderClear
	{
		None  = 0,
		Color = 1 << 0,
		Depth = 1 << 1,
		All   = Color|Depth
	}

	/// <summary>What type of device is the source of the pointer? This is a bit-flag that can 
	/// contain some input source family information.</summary>
	[Flags]
	public enum InputSource
	{
		/// <summary>Matches with all input sources!</summary>
		Any        = 0x7FFFFFFF,
		/// <summary>Matches with any hand input source.</summary>
		Hand       = 1 << 0,
		/// <summary>Matches with left hand input sources.</summary>
		HandLeft   = 1 << 1,
		/// <summary>Matches with right hand input sources.</summary>
		HandRight  = 1 << 2,
		/// <summary>Matches with Gaze category input sources.</summary>
		Gaze       = 1 << 4,
		/// <summary>Matches with the head gaze input source.</summary>
		GazeHead   = 1 << 5,
		/// <summary>Matches with the eye gaze input source.</summary>
		GazeEyes   = 1 << 6,
		/// <summary>Matches with mouse cursor simulated gaze as an input source.</summary>
		GazeCursor = 1 << 7,
		/// <summary>Matches with any input source that has an activation button!</summary>
		CanPress   = 1 << 8,
	}

	/// <summary>A bit-flag for the current state of a button input.</summary>
	[Flags]
	public enum BtnState
	{
		/// <summary>Is the button currently up, unpressed?</summary>
		Inactive     = 0,
		/// <summary>Is the button currently down, pressed?</summary>
		Active       = 1 << 0,
		/// <summary>Has the button just been released? Only true for a single frame.</summary>
		JustInactive = 1 << 1,
		/// <summary>Has the button just been pressed? Only true for a single frame.</summary>
		JustActive   = 1 << 2,
		/// <summary>Has the button just changed state this frame?</summary>
		Changed      = JustInactive | JustActive,
		/// <summary>Matches with all states!</summary>
		Any          = 0x7FFFFFFF,

	}

	/// <summary>This is the tracking state of a sensory input in the world,
	/// like a controller's position sensor, or a QR code identified by a 
	/// tracking system.</summary>
	public enum TrackState
	{
		/// <summary>The system has no current knowledge about the state of
		/// this input. It may be out of visibility, or possibly just
		/// disconnected.</summary>
		Lost     = 0,
		/// <summary>The system doesn't know for sure where this is, but it
		/// has an educated guess that may be inferred from previous data at
		/// a lower quality. For example, a controller may still have 
		/// accelerometer data after going out of view, which can still be
		/// accurate for a short time after losing optical tracking.</summary>
		Inferred = 1,
		/// <summary>The system actively knows where this input is. Within
		/// the constraints of the relevant hardware's capabilities, this is
		/// as accurate as it gets!</summary>
		Known    = 2
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
		/// <summary>What input soure did this pointer come from? This is
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

	/// <summary>An enum for indicating which hand to use!</summary>
	public enum Handed
	{
		/// <summary>Left hand.</summary>
		Left = 0,
		/// <summary>Right hand.</summary>
		Right,
		/// <summary>The number of hands one generally has, this is much nicer than doing
		/// a for loop with '2' as the condition! It's much clearer when you can loop Hand.Max
		/// times instead.</summary>
		Max,
	}

	/// <summary>Severity of a log item.</summary>
	public enum LogLevel
	{
		None,
		/// <summary>This is for diagnostic information, where you need to know details about what -exactly-
		/// is going on in the system. This info doesn't surface by default.</summary>
		Diagnostic = 1,
		/// <summary>This is non-critical information, just to let you know what's going on.</summary>
		Info,
		/// <summary>Something bad has happened, but it's still within the realm of what's expected.</summary>
		Warning,
		/// <summary>Danger Will Robinson! Something really bad just happened and needs fixing!</summary>
		Error
	}

	enum LogColors
	{
		Ansi,
		None
	}

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void LogCallback(LogLevel level, string text);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate float AudioGenerator(float time);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	internal delegate void PickerCallback(IntPtr callback_data, int confirmed, string filename);

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
	}

	/// <summary>A description of what type of window to draw! This is a bit
	/// flag, so it can contain multiple elements.</summary>
	[Flags]
	public enum UIWin
	{
		/// <summary>A normal window has a head and a body to it. Both can be
		/// grabbed.</summary>
		Normal = Head | Body,
		/// <summary>No body, no head, none of the window will be grabbable.
		/// Not really a flag, just set to this value.</summary>
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
		None,
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
	}
}