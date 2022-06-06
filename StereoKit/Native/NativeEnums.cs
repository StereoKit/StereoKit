// This is a generated file based on stereokit.h! Please don't modify it
// directly :) Instead, modify the header file, and run the StereoKitAPIGen
// project.

using System;

namespace StereoKit
{
	/// <summary>Specifies a type of display mode StereoKit uses, like
	/// Mixed Reality headset display vs. a PC display, or even just
	/// rendering to an offscreen surface, or not rendering at all!</summary>
	public enum DisplayMode {
		/// <summary>Creates an OpenXR instance, and drives display/input
		/// through that.</summary>
		MixedReality = 0,
		/// <summary>Creates a flat, Win32 window, and simulates some MR
		/// functionality. Great for debugging.</summary>
		Flatscreen   = 1,
		/// <summary>Not tested yet, but this is meant to run StereoKit
		/// without rendering to any display at all. This would allow for
		/// rendering to textures, running a server that can do MR related
		/// tasks, etc.</summary>
		None         = 2,
	}

	/// <summary>This is used to determine what kind of depth buffer
	/// StereoKit uses!</summary>
	public enum DepthMode {
		/// <summary>Default mode, uses 16 bit on mobile devices like
		/// HoloLens and Quest, and 32 bit on higher powered platforms like
		/// PC. If you need a far view distance even on mobile devices,
		/// prefer D32 or Stencil instead.</summary>
		Balanced     = 0,
		/// <summary>16 bit depth buffer, this is fast and recommended for
		/// devices like the HoloLens. This is especially important for fast
		/// depth based reprojection. Far view distances will suffer here
		/// though, so keep your clipping far plane as close as possible.</summary>
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

	/// <summary>TODO: remove this in v0.4
	/// This describes the type of display tech used on a Mixed
	/// Reality device. This will be replaced by `DisplayBlend` in v0.4.</summary>
	[Flags]
	public enum Display {
		/// <summary>Default value, when using this as a search type, it will
		/// fall back to default behavior which defers to platform
		/// preference.</summary>
		None         = 0,
		/// <summary>This display is opaque, with no view into the real world!
		/// This is equivalent to a VR headset, or a PC screen.</summary>
		Opaque       = 1 << 0,
		/// <summary>This display is transparent, and adds light on top of
		/// the real world. This is equivalent to a HoloLens type of device.</summary>
		Additive     = 1 << 1,
		/// <summary>This is a physically opaque display, but with a camera
		/// passthrough displaying the world behind it anyhow. This would be
		/// like a Varjo XR-1, or phone-camera based AR.</summary>
		Blend        = 1 << 2,
		/// <summary>Use Display.Blend instead, to be removed in v0.4</summary>
		Passthrough  = 1 << 2,
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
	[Flags]
	public enum DisplayBlend {
		/// <summary>Default value, when using this as a search type, it will
		/// fall back to default behavior which defers to platform
		/// preference.</summary>
		None         = 0,
		/// <summary>This display is opaque, with no view into the real world!
		/// This is equivalent to a VR headset, or a PC screen.</summary>
		Opaque       = 1 << 0,
		/// <summary>This display is transparent, and adds light on top of
		/// the real world. This is equivalent to a HoloLens type of device.</summary>
		Additive     = 1 << 1,
		/// <summary>This is a physically opaque display, but with a camera
		/// passthrough displaying the world behind it anyhow. This would be
		/// like a Varjo XR-1, or phone-camera based AR.</summary>
		Blend        = 1 << 2,
		/// <summary>This matches either transparent display type! Additive
		/// or Blend. For use when you just want to see the world behind your
		/// application.</summary>
		AnyTransparent = Additive | Blend,
	}

	/// <summary>Severity of a log item.</summary>
	public enum LogLevel {
		None         = 0,
		/// <summary>This is for diagnostic information, where you need to know
		/// details about what -exactly- is going on in the system. This
		/// info doesn't surface by default.</summary>
		Diagnostic,
		/// <summary>This is non-critical information, just to let you know what's
		/// going on.</summary>
		Info,
		/// <summary>Something bad has happened, but it's still within the realm of
		/// what's expected.</summary>
		Warning,
		/// <summary>Danger Will Robinson! Something really bad just happened and
		/// needs fixing!</summary>
		Error,
	}

	/// <summary>When rendering content, you can filter what you're rendering by the
	/// RenderLayer that they're on. This allows you to draw items that are
	/// visible in one render, but not another. For example, you may wish
	/// to draw a player's avatar in a 'mirror' rendertarget, but not in
	/// the primary display. See `Renderer.LayerFilter` for configuring what
	/// the primary display renders.</summary>
	[Flags]
	public enum RenderLayer {
		/// <summary>The default render layer. All Draw use this layer unless
		/// otherwise specified.</summary>
		Layer0       = 1 << 0,
		/// <summary>Render layer 1.</summary>
		Layer1       = 1 << 1,
		/// <summary>Render layer 2.</summary>
		Layer2       = 1 << 2,
		/// <summary>Render layer 3.</summary>
		Layer3       = 1 << 3,
		/// <summary>Render layer 4.</summary>
		Layer4       = 1 << 4,
		/// <summary>Render layer 5.</summary>
		Layer5       = 1 << 5,
		/// <summary>Render layer 6.</summary>
		Layer6       = 1 << 6,
		/// <summary>Render layer 7.</summary>
		Layer7       = 1 << 7,
		/// <summary>Render layer 8.</summary>
		Layer8       = 1 << 8,
		/// <summary>Render layer 9.</summary>
		Layer9       = 1 << 9,
		/// <summary>The default VFX layer, StereoKit draws some non-standard
		/// mesh content using this flag, such as lines.</summary>
		Vfx          = 1 << 10,
		/// <summary>This is a flag that specifies all possible layers. If you
		/// want to render all layers, then this is the layer filter
		/// you would use. This is the default for render filtering.</summary>
		All          = 0xFFFF,
		/// <summary>This is a combination of all layers that are not the VFX layer.</summary>
		AllRegular   = Layer0 | Layer1 | Layer2 | Layer3 | Layer4 | Layer5 | Layer6 | Layer7 | Layer8 | Layer9,
	}

	/// <summary>This tells about the app's current focus state, whether it's active and
	/// receiving input, or if it's backgrounded or hidden. This can be
	/// important since apps may still run and render when unfocused, as the app
	/// may still be visible behind the app that _does_ have focus.</summary>
	public enum AppFocus {
		/// <summary>This StereoKit app is active, focused, and receiving input from the
		/// user. Application should behave as normal.</summary>
		Active,
		/// <summary>This StereoKit app has been unfocused, something may be compositing
		/// on top of the app such as an OS dashboard. The app is still visible,
		/// but some other thing has focus and is receiving input. You may wish
		/// to pause, disable input tracking, or other such things.</summary>
		Background,
		/// <summary>This app is not rendering currently.</summary>
		Hidden,
	}

	/// <summary>StereoKit uses an asynchronous loading system to prevent assets from
	/// blocking execution! This means that asset loading systems will return
	/// an asset to you right away, even though it is still being processed
	/// in the background.</summary>
	public enum AssetState {
		/// <summary>This asset encountered an issue when parsing the source data. Either
		/// the format is unrecognized by StereoKit, or the data may be corrupt.
		/// Check the logs for additional details.</summary>
		ErrorUnsupported = -3,
		/// <summary>The asset data was not found! This is most likely an issue with a
		/// bad file path, or file permissions. Check the logs for additional
		/// details.</summary>
		ErrorNotFound = -2,
		/// <summary>An unknown error occurred when trying to load the asset! Check the
		/// logs for additional details.</summary>
		Error        = -1,
		/// <summary>This asset is in its default state. It has not been told to load
		/// anything, nor does it have any data!</summary>
		None         = 0,
		/// <summary>This asset is currently queued for loading, but hasn't received any
		/// data yet. Attempting to access metadata or asset data will result in
		/// blocking the app's execution until that data is loaded!</summary>
		Loading,
		/// <summary>This asset is still loading, but some of the higher level data is
		/// already available for inspection without blocking the app.
		/// Attempting to access the core asset data will result in blocking the
		/// app's execution until that data is loaded!</summary>
		LoadedMeta,
		/// <summary>This asset is completely loaded without issues, and is ready for
		/// use!</summary>
		Loaded,
	}

	/// <summary>For performance sensitive areas, or places dealing with large chunks of
	/// memory, it can be faster to get a reference to that memory rather than
	/// copying it! However, if this isn't explicitly stated, it isn't necessarily
	/// clear what's happening. So this enum allows us to visibly specify what type
	/// of memory reference is occurring.</summary>
	public enum Memory {
		/// <summary>The chunk of memory involved here is a reference that is still managed or
		/// used by StereoKit! You should _not_ free it, and be extremely cautious
		/// about modifying it.</summary>
		Reference,
		/// <summary>This memory is now _yours_ and you must free it yourself! Memory has been
		/// allocated, and the data has been copied over to it. Pricey! But safe.</summary>
		Copy,
	}

	/// <summary>Textures come in various types and flavors! These are bit-flags
	/// that tell StereoKit what type of texture we want, and how the application
	/// might use it!</summary>
	[Flags]
	public enum TexType {
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
		Image        = ImageNomips | Mips,
	}

	/// <summary>What type of color information will the texture contain? A
	/// good default here is Rgba32.</summary>
	public enum TexFormat {
		/// <summary>A default zero value for TexFormat! Uninitialized formats
		/// will get this value and **** **** up so you know to assign it
		/// properly :)</summary>
		None         = 0,
		/// <summary>Red/Green/Blue/Transparency data channels, at 8 bits
		/// per-channel in sRGB color space. This is what you'll want most of
		/// the time you're dealing with color images! Matches well with the
		/// Color32 struct! If you're storing normals, rough/metal, or
		/// anything else, use Rgba32Linear.</summary>
		Rgba32       = 1,
		/// <summary>Red/Green/Blue/Transparency data channels, at 8 bits
		/// per-channel in linear color space. This is what you'll want most
		/// of the time you're dealing with color data! Matches well with the
		/// Color32 struct.</summary>
		Rgba32Linear = 2,
		/// <summary>Red/Green/Blue/Transparency data channels, at 8 bits
		/// per-channel in linear color space. This is what you'll want most
		/// of the time you're dealing with color data! Matches well with the
		/// Color32 struct.</summary>
		Bgra32       = 3,
		/// <summary>Red/Green/Blue/Transparency data channels, at 8 bits
		/// per-channel in linear color space. This is what you'll want most
		/// of the time you're dealing with color data! Matches well with the
		/// Color32 struct.</summary>
		Bgra32Linear = 4,
		/// <summary>Red/Green/Blue/Transparency data channels, at 8 bits
		/// per-channel in linear color space. This is what you'll want most
		/// of the time you're dealing with color data! Matches well with the
		/// Color32 struct.</summary>
		Rg11b10      = 5,
		/// <summary>Red/Green/Blue/Transparency data channels, at 8 bits
		/// per-channel in linear color space. This is what you'll want most
		/// of the time you're dealing with color data! Matches well with the
		/// Color32 struct.</summary>
		Rgb10a2      = 6,
		/// <summary>TODO: remove during major version update</summary>
		Rgba64       = 7,
		Rgba64s      = 8,
		Rgba64f      = 9,
		/// <summary>Red/Green/Blue/Transparency data channels at 32 bits
		/// per-channel! Basically 4 floats per color, which is bonkers
		/// expensive. Don't use this unless you know -exactly- what you're
		/// doing.</summary>
		Rgba128      = 10,
		/// <summary>A single channel of data, with 8 bits per-pixel! This
		/// can be great when you're only using one channel, and want to
		/// reduce memory usage. Values in the shader are always 0.0-1.0.</summary>
		R8           = 11,
		/// <summary>A single channel of data, with 16 bits per-pixel! This
		/// is a good format for height maps, since it stores a fair bit of
		/// information in it. Values in the shader are always 0.0-1.0.</summary>
		R16          = 12,
		/// <summary>A single channel of data, with 32 bits per-pixel! This
		/// basically treats each pixel as a generic float, so you can do all
		/// sorts of strange and interesting things with this.</summary>
		R32          = 13,
		/// <summary>A depth data format, 24 bits for depth data, and 8 bits
		/// to store stencil information! Stencil data can be used for things
		/// like clipping effects, deferred rendering, or shadow effects.</summary>
		DepthStencil = 14,
		/// <summary>32 bits of data per depth value! This is pretty detailed,
		/// and is excellent for experiences that have a very far view
		/// distance.</summary>
		Depth32      = 15,
		/// <summary>16 bits of depth is not a lot, but it can be enough if
		/// your far clipping plane is pretty close. If you're seeing lots of
		/// flickering where two objects overlap, you either need to bring
		/// your far clip in, or switch to 32/24 bit depth.</summary>
		Depth16      = 16,
		/// <summary>16 bits of depth is not a lot, but it can be enough if
		/// your far clipping plane is pretty close. If you're seeing lots of
		/// flickering where two objects overlap, you either need to bring
		/// your far clip in, or switch to 32/24 bit depth.</summary>
		Rgba64u      = Rgba64,
	}

	/// <summary>How does the shader grab pixels from the texture? Or more
	/// specifically, how does the shader grab colors between the provided
	/// pixels? If you'd like an in-depth explanation of these topics, check
	/// out [this exploration of texture filtering](https://medium.com/@bgolus/sharper-mipmapping-using-shader-based-supersampling-ed7aadb47bec)
	/// by graphics wizard Ben Golus.</summary>
	public enum TexSample {
		/// <summary>Use a linear blend between adjacent pixels, this creates
		/// a smooth, blurry look when texture resolution is too low.</summary>
		Linear       = 0,
		/// <summary>Choose the nearest pixel's color! This makes your texture
		/// look like pixel art if you're too close.</summary>
		Point,
		/// <summary>This helps reduce texture blurriness when a surface is
		/// viewed at an extreme angle!</summary>
		Anisotropic,
	}

	/// <summary>What happens when the shader asks for a texture coordinate
	/// that's outside the texture?? Believe it or not, this happens plenty
	/// often!</summary>
	public enum TexAddress {
		/// <summary>Wrap the UV coordinate around to the other side of the
		/// texture! This is basically like a looping texture, and is an
		/// excellent default. If you can see weird bits of color at the edges
		/// of your texture, this may be due to Wrap blending the color with
		/// the other side of the texture, Clamp may be better in such cases.</summary>
		Wrap         = 0,
		/// <summary>Clamp the UV coordinates to the edge of the texture!
		/// This'll create color streaks that continue to forever. This is
		/// actually really great for non-looping textures that you know will
		/// always be accessed on the 0-1 range.</summary>
		Clamp,
		/// <summary>Like Wrap, but it reflects the image each time! Who needs
		/// this? I'm not sure!! But the graphics card can do it, so now you
		/// can too!</summary>
		Mirror,
	}

	/// <summary>Also known as 'alpha' for those in the know. But there's
	/// actually more than one type of transparency in rendering! The
	/// horrors. We're keepin' it fairly simple for now, so you get three
	/// options!</summary>
	public enum Transparency {
		/// <summary>Not actually transparent! This is opaque! Solid! It's
		/// the default option, and it's the fastest option! Opaque objects
		/// write to the z-buffer, the occlude pixels behind them, and they
		/// can be used as input to important Mixed Reality features like
		/// Late Stage Reprojection that'll make your view more stable!</summary>
		None         = 1,
		/// <summary>This will blend with the pixels behind it. This is 
		/// transparent! You may not want to write to the z-buffer, and it's
		/// slower than opaque materials.</summary>
		Blend,
		/// <summary>This will straight up add the pixel color to the color
		/// buffer! This usually looks -really- glowy, so it makes for good
		/// particles or lighting effects.</summary>
		Add,
	}

	/// <summary>Culling is discarding an object from the render pipeline!
	/// This enum describes how mesh faces get discarded on the graphics
	/// card. With culling set to none, you can double the number of pixels
	/// the GPU ends up drawing, which can have a big impact on performance.
	/// None can be appropriate in cases where the mesh is designed to be
	/// 'double sided'. Front can also be helpful when you want to flip a
	/// mesh 'inside-out'!</summary>
	public enum Cull {
		/// <summary>Discard if the back of the triangle face is pointing
		/// towards the camera. This is the default behavior.</summary>
		Back         = 0,
		/// <summary>Discard if the front of the triangle face is pointing
		/// towards the camera. This is opposite the default behavior.</summary>
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
	public enum DepthTest {
		/// <summary>Default behavior, pixels behind the depth buffer will be
		/// discarded, and pixels in front of it will be drawn.</summary>
		Less         = 0,
		/// <summary>Pixels behind the depth buffer will be discarded, and
		/// pixels in front of, or at the depth buffer's value it will be
		/// drawn. This could be great for things that might be sitting
		/// exactly on a floor or wall.</summary>
		LessOrEq,
		/// <summary>Pixels in front of the zbuffer will be discarded! This
		/// is opposite of how things normally work. Great for drawing
		/// indicators that something is occluded by a wall or other
		/// geometry.</summary>
		Greater,
		/// <summary>Pixels in front of (or exactly at) the zbuffer will be
		/// discarded! This is opposite of how things normally work. Great
		/// for drawing indicators that something is occluded by a wall or
		/// other geometry.</summary>
		GreaterOrEq,
		/// <summary>Only draw pixels if they're at exactly the same depth as
		/// the zbuffer!</summary>
		Equal,
		/// <summary>Draw any pixel that's not exactly at the value in the
		/// zbuffer.</summary>
		NotEqual,
		/// <summary>Don't look at the zbuffer at all, just draw everything,
		/// always, all the time! At this point, the order at which the mesh
		/// gets drawn will be  super important, so don't forget about
		/// `Material.QueueOffset`!</summary>
		Always,
		/// <summary>Never draw a pixel, regardless of what's in the zbuffer.
		/// I can think of better ways to do this, but uhh, this is here for
		/// completeness! Maybe you can find a use for it.</summary>
		Never,
	}

	/// <summary>TODO: v0.4 This may need significant revision?
	/// What type of data does this material parameter need? This is
	/// used to tell the shader how large the data is, and where to attach it
	/// to on the shader.</summary>
	public enum MaterialParam {
		/// <summary>This data type is not currently recognized. Please
		/// report your case on GitHub Issues!</summary>
		Unknown      = 0,
		/// <summary>A single 32 bit float value.</summary>
		Float        = 1,
		/// <summary>A color value described by 4 floating point values. Memory-wise this is
		/// the same as a Vector4, but in the shader this variable has a ':color'
		/// tag applied to it using StereoKits's shader info syntax, indicating it's
		/// a color value. Color values for shaders should be in linear space, not
		/// gamma.</summary>
		Color128     = 2,
		/// <summary>A 2 component vector composed of floating point values.</summary>
		Vector2      = 3,
		/// <summary>A 3 component vector composed of floating point values.</summary>
		Vector3      = 4,
		/// <summary>A 4 component vector composed of floating point values.</summary>
		Vector4      = 5,

		/// <summary>A 4 component vector composed of floating point values.
		/// TODO: Remove in v0.4</summary>
		[Obsolete("Replaced by MaterialParam.Vector4")]
		Vector       = 5,
		/// <summary>A 4x4 matrix of floats.</summary>
		Matrix       = 6,
		/// <summary>Texture information!</summary>
		Texture      = 7,
		/// <summary>A 1 component vector composed of signed integers.</summary>
		Int          = 8,
		/// <summary>A 2 component vector composed of signed integers.</summary>
		Int2         = 9,
		/// <summary>A 3 component vector composed of signed integers.</summary>
		Int3         = 10,
		/// <summary>A 4 component vector composed of signed integers.</summary>
		Int4         = 11,
		/// <summary>A 1 component vector composed of unsigned integers. This may also be a
		/// boolean.</summary>
		UInt         = 12,
		/// <summary>A 2 component vector composed of unsigned integers.</summary>
		UInt2        = 13,
		/// <summary>A 3 component vector composed of unsigned integers.</summary>
		UInt3        = 14,
		/// <summary>A 4 component vector composed of unsigned integers.</summary>
		UInt4        = 15,
	}

	/// <summary>This enum describes how text layout behaves within the space
	/// it is given.</summary>
	[Flags]
	public enum TextFit {
		/// <summary>The text will wrap around to the next line down when it
		/// reaches the end of the space on the X axis.</summary>
		Wrap         = 1 << 0,
		/// <summary>When the text reaches the end, it is simply truncated
		/// and no longer visible.</summary>
		Clip         = 1 << 1,
		/// <summary>If the text is too large to fit in the space provided,
		/// it will be scaled down to fit inside. This will not scale up.</summary>
		Squeeze      = 1 << 2,
		/// <summary>If the text is larger, or smaller than the space 
		/// provided, it will scale down or up to fill the space.</summary>
		Exact        = 1 << 3,
		/// <summary>The text will ignore the containing space, and just keep
		/// on going.</summary>
		Overflow     = 1 << 4,
	}

	/// <summary>A bit-flag enum for describing alignment or positioning.
	/// Items can be combined using the '|' operator, like so:</summary>
	[Flags]
	public enum TextAlign {
		/// <summary>On the x axis, this item should start on the left.</summary>
		XLeft        = 1 << 0,
		/// <summary>On the y axis, this item should start at the top.</summary>
		YTop         = 1 << 1,
		/// <summary>On the x axis, the item should be centered.</summary>
		XCenter      = 1 << 2,
		/// <summary>On the y axis, the item should be centered.</summary>
		YCenter      = 1 << 3,
		/// <summary>On the x axis, this item should start on the right.</summary>
		XRight       = 1 << 4,
		/// <summary>On the y axis, this item should start on the bottom.</summary>
		YBottom      = 1 << 5,
		/// <summary>Center on both X and Y axes. This is a combination of 
		/// XCenter and YCenter.</summary>
		Center       = XCenter | YCenter,
		/// <summary>Start on the left of the X axis, center on the Y axis. 
		/// This is a combination of XLeft and YCenter.</summary>
		CenterLeft   = XLeft | YCenter,
		/// <summary>Start on the right of the X axis, center on the Y axis. 
		/// This is a combination of XRight and YCenter.</summary>
		CenterRight  = XRight | YCenter,
		/// <summary>Center on the X axis, and top on the Y axis. This is a
		/// combination of XCenter and YTop.</summary>
		TopCenter    = XCenter | YTop,
		/// <summary>Start on the left of the X axis, and top on the Y axis.
		/// This is a combination of XLeft and YTop.</summary>
		TopLeft      = XLeft | YTop,
		/// <summary>Start on the right of the X axis, and top on the Y axis.
		/// This is a combination of XRight and YTop.</summary>
		TopRight     = XRight | YTop,
		/// <summary>Center on the X axis, and bottom on the Y axis. This is
		/// a combination of XCenter and YBottom.</summary>
		BottomCenter = XCenter | YBottom,
		/// <summary>Start on the left of the X axis, and bottom on the Y
		/// axis. This is a combination of XLeft and YBottom.</summary>
		BottomLeft   = XLeft | YBottom,
		/// <summary>Start on the right of the X axis, and bottom on the Y
		/// axis.This is a combination of XRight and YBottom.</summary>
		BottomRight  = XRight | YBottom,
	}

	/// <summary>This describes the behavior of a 'Solid' physics object! The
	/// physics engine will apply forces differently based on this type.</summary>
	public enum SolidType {
		/// <summary>This object behaves like a normal physical object, it'll
		/// fall, get pushed around, and generally be susceptible to physical
		/// forces! This is a 'Dynamic' body in physics simulation terms.</summary>
		Normal       = 0,
		/// <summary>Immovable objects are always stationary! They have
		/// infinite mass, zero velocity, and can't collide with Immovable of
		/// Unaffected types.</summary>
		Immovable,
		/// <summary>Unaffected objects have infinite mass, but can have a
		/// velocity! They'll move under their own forces, but nothing in the
		/// simulation will affect them. They don't collide with Immovable or
		/// Unaffected types.</summary>
		Unaffected,
	}

	/// <summary>Describes how an animation is played back, and what to do when
	/// the animation hits the end.</summary>
	public enum AnimMode {
		/// <summary>If the animation reaches the end, it will always loop
		/// back around to the start again.</summary>
		Loop,
		/// <summary>When the animation reaches the end, it will freeze
		/// in-place.</summary>
		Once,
		/// <summary>The animation will not progress on its own, and instead
		/// must be driven by providing information to the model's AnimTime
		/// or AnimCompletion properties.</summary>
		Manual,
	}

	/// <summary>The way the Sprite is stored on the backend! Does it get
	/// batched and atlased for draw efficiency, or is it a single image?</summary>
	public enum SpriteType {
		/// <summary>The sprite will be batched onto an atlas texture so all
		/// sprites can be drawn in a single pass. This is excellent for
		/// performance! The only thing to watch out for here, adding a sprite
		/// to an atlas will rebuild the atlas texture! This can be a bit
		/// expensive, so it's recommended to add all sprites to an atlas at
		/// start, rather than during runtime. Also, if an image is too large,
		/// it may take up too much space on the atlas, and may be better as a
		/// Single sprite type.</summary>
		Atlased      = 0,
		/// <summary>This sprite is on its own texture. This is best for large
		/// images, items that get loaded and unloaded during runtime, or for
		/// sprites that may have edge artifacts or severe 'bleed' from
		/// adjacent atlased images.</summary>
		Single,
	}

	/// <summary>When rendering to a rendertarget, this tells if and what of the
	/// rendertarget gets cleared before rendering. For example, if you
	/// are assembling a sheet of images, you may want to clear
	/// everything on the first image draw, but not clear on subsequent
	/// draws.</summary>
	[Flags]
	public enum RenderClear {
		/// <summary>Don't clear anything, leave it as it is.</summary>
		None         = 0,
		/// <summary>Clear the rendertarget's color data.</summary>
		Color        = 1 << 0,
		/// <summary>Clear the rendertarget's depth data, if present.</summary>
		Depth        = 1 << 1,
		/// <summary>Clear both color and depth data.</summary>
		All          = Color | Depth,
	}

	/// <summary>The projection mode used by StereoKit for the main camera! You
	/// can use this with Renderer.Projection. These options are only
	/// available in flatscreen mode, as MR headsets provide very
	/// specific projection matrices.</summary>
	public enum Projection {
		/// <summary>This is the default projection mode, and the one you're most likely
		/// to be familiar with! This is where parallel lines will converge as
		/// they go into the distance.</summary>
		Perspective  = 0,
		/// <summary>Orthographic projection mode is often used for tools, 2D rendering,
		/// thumbnails of 3D objects, or other similar cases. In this mode,
		/// parallel lines remain parallel regardless of how far they travel.</summary>
		Ortho        = 1,
	}

	/// <summary>When opening the Platform.FilePicker, this enum describes
	/// how the picker should look and behave.</summary>
	public enum PickerMode {
		/// <summary>Allow opening a single file.</summary>
		Open,
		/// <summary>Allow the user to enter or select the name of the
		/// destination file.</summary>
		Save,
	}

	/// <summary>Soft keyboard layouts are often specific to the type of text that they're
	/// editing! This enum is a collection of common text contexts that SK can pass
	/// along to the OS's soft keyboard for a more optimal layout.</summary>
	public enum TextContext {
		/// <summary>General text editing, this is the most common type of text, and would
		/// result in a 'standard' keyboard layout.</summary>
		Text         = 1,
		/// <summary>Numbers and numerical values.</summary>
		Number       = 2,
		/// <summary>This text specifically represents some kind of URL/URI address.</summary>
		Uri          = 10,
		/// <summary>This is a password, and should not be visible when typed!</summary>
		Password     = 18,
	}

	/// <summary>What type of device is the source of the pointer? This is a
	/// bit-flag that can contain some input source family information.</summary>
	[Flags]
	public enum InputSource {
		/// <summary>Matches with all input sources!</summary>
		Any          = 0x7FFFFFFF,
		/// <summary>Matches with any hand input source.</summary>
		Hand         = 1 << 0,
		/// <summary>Matches with left hand input sources.</summary>
		HandLeft     = 1 << 1,
		/// <summary>Matches with right hand input sources.</summary>
		HandRight    = 1 << 2,
		/// <summary>Matches with Gaze category input sources.</summary>
		Gaze         = 1 << 4,
		/// <summary>Matches with the head gaze input source.</summary>
		GazeHead     = 1 << 5,
		/// <summary>Matches with the eye gaze input source.</summary>
		GazeEyes     = 1 << 6,
		/// <summary>Matches with mouse cursor simulated gaze as an input source.</summary>
		GazeCursor   = 1 << 7,
		/// <summary>Matches with any input source that has an activation button!</summary>
		CanPress     = 1 << 8,
	}

	/// <summary>An enum for indicating which hand to use!</summary>
	public enum Handed {
		/// <summary>Left hand.</summary>
		Left         = 0,
		/// <summary>Right hand.</summary>
		Right        = 1,
		/// <summary>The number of hands one generally has, this is much nicer
		/// than doing a for loop with '2' as the condition! It's much clearer
		/// when you can loop Hand.Max times instead.</summary>
		Max          = 2,
	}

	/// <summary>A bit-flag for the current state of a button input.</summary>
	[Flags]
	public enum BtnState {
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
	public enum TrackState {
		/// <summary>The system has no current knowledge about the state of
		/// this input. It may be out of visibility, or possibly just
		/// disconnected.</summary>
		Lost         = 0,
		/// <summary>The system doesn't know for sure where this is, but it
		/// has an educated guess that may be inferred from previous data at
		/// a lower quality. For example, a controller may still have
		/// accelerometer data after going out of view, which can still be
		/// accurate for a short time after losing optical tracking.</summary>
		Inferred     = 1,
		/// <summary>The system actively knows where this input is. Within
		/// the constraints of the relevant hardware's capabilities, this is
		/// as accurate as it gets!</summary>
		Known        = 2,
	}

	/// <summary>A collection of system key codes, representing keyboard
	/// characters and mouse buttons. Based on VK codes.</summary>
	public enum Key {
		/// <summary>Doesn't represent a key, generally means this item has not been set to
		/// any particular value!</summary>
		None         = 0x00,
		/// <summary>Left mouse button.</summary>
		MouseLeft    = 0x01,
		/// <summary>Right mouse button.</summary>
		MouseRight   = 0x02,
		/// <summary>Center mouse button.</summary>
		MouseCenter  = 0x04,
		/// <summary>Mouse forward button.</summary>
		MouseForward = 0x05,
		/// <summary>Mouse back button.</summary>
		MouseBack    = 0x06,
		/// <summary>Backspace</summary>
		Backspace    = 0x08,
		/// <summary>Tab</summary>
		Tab          = 0x09,
		/// <summary>Return, or Enter.</summary>
		Return       = 0x0D,
		/// <summary>Left or right Shift.</summary>
		Shift        = 0x10,
		/// <summary>Left or right Control key.</summary>
		Ctrl         = 0x11,
		/// <summary>Left or right Alt key.</summary>
		Alt          = 0x12,
		/// <summary>This behaves a little differently! This tells the toggle
		/// state of caps lock, rather than the key state itself.</summary>
		CapsLock     = 0x14,
		/// <summary>Escape</summary>
		Esc          = 0x1B,
		/// <summary>Space</summary>
		Space        = 0x20,
		/// <summary>End</summary>
		End          = 0x23,
		/// <summary>Home</summary>
		Home         = 0x24,
		/// <summary>Left arrow key.</summary>
		Left         = 0x25,
		/// <summary>Right arrow key.</summary>
		Right        = 0x27,
		/// <summary>Up arrow key.</summary>
		Up           = 0x26,
		/// <summary>Down arrow key.</summary>
		Down         = 0x28,
		/// <summary>Page up</summary>
		PageUp       = 0x21,
		/// <summary>Page down</summary>
		PageDown     = 0x22,
		/// <summary>Printscreen</summary>
		Printscreen  = 0x2A,
		/// <summary>Any Insert key.</summary>
		Insert       = 0x2D,
		/// <summary>Any Delete key.</summary>
		Del          = 0x2E,
		/// <summary>Keyboard top row 0, with shift is ')'.</summary>
		N0           = 0x30,
		/// <summary>Keyboard top row 1, with shift is '!'.</summary>
		N1           = 0x31,
		/// <summary>Keyboard top row 2, with shift is '@'.</summary>
		N2           = 0x32,
		/// <summary>Keyboard top row 3, with shift is '#'.</summary>
		N3           = 0x33,
		/// <summary>Keyboard top row 4, with shift is '$'.</summary>
		N4           = 0x34,
		/// <summary>Keyboard top row 5, with shift is '%'.</summary>
		N5           = 0x35,
		/// <summary>Keyboard top row 6, with shift is '^'.</summary>
		N6           = 0x36,
		/// <summary>Keyboard top row 7, with shift is '&amp;'.</summary>
		N7           = 0x37,
		/// <summary>Keyboard top row 8, with shift is '*'.</summary>
		N8           = 0x38,
		/// <summary>Keyboard top row 9, with shift is '('.</summary>
		N9           = 0x39,
		/// <summary>a/A</summary>
		A            = 0x41,
		/// <summary>b/B</summary>
		B            = 0x42,
		/// <summary>c/C</summary>
		C            = 0x43,
		/// <summary>d/D</summary>
		D            = 0x44,
		/// <summary>e/E</summary>
		E            = 0x45,
		/// <summary>f/F</summary>
		F            = 0x46,
		/// <summary>g/G</summary>
		G            = 0x47,
		/// <summary>h/H</summary>
		H            = 0x48,
		/// <summary>i/I</summary>
		I            = 0x49,
		/// <summary>j/J</summary>
		J            = 0x4A,
		/// <summary>k/K</summary>
		K            = 0x4B,
		/// <summary>l/L</summary>
		L            = 0x4C,
		/// <summary>m/M</summary>
		M            = 0x4D,
		/// <summary>n/N</summary>
		N            = 0x4E,
		/// <summary>o/O</summary>
		O            = 0x4F,
		/// <summary>p/P</summary>
		P            = 0x50,
		/// <summary>q/Q</summary>
		Q            = 0x51,
		/// <summary>r/R</summary>
		R            = 0x52,
		/// <summary>s/S</summary>
		S            = 0x53,
		/// <summary>t/T</summary>
		T            = 0x54,
		/// <summary>u/U</summary>
		U            = 0x55,
		/// <summary>v/V</summary>
		V            = 0x56,
		/// <summary>w/W</summary>
		W            = 0x57,
		/// <summary>x/X</summary>
		X            = 0x58,
		/// <summary>y/Y</summary>
		Y            = 0x59,
		/// <summary>z/Z</summary>
		Z            = 0x5A,
		/// <summary>0 on the numpad, when numlock is on.</summary>
		Num0         = 0x60,
		/// <summary>1 on the numpad, when numlock is on.</summary>
		Num1         = 0x61,
		/// <summary>2 on the numpad, when numlock is on.</summary>
		Num2         = 0x62,
		/// <summary>3 on the numpad, when numlock is on.</summary>
		Num3         = 0x63,
		/// <summary>4 on the numpad, when numlock is on.</summary>
		Num4         = 0x64,
		/// <summary>5 on the numpad, when numlock is on.</summary>
		Num5         = 0x65,
		/// <summary>6 on the numpad, when numlock is on.</summary>
		Num6         = 0x66,
		/// <summary>7 on the numpad, when numlock is on.</summary>
		Num7         = 0x67,
		/// <summary>8 on the numpad, when numlock is on.</summary>
		Num8         = 0x68,
		/// <summary>9 on the numpad, when numlock is on.</summary>
		Num9         = 0x69,
		/// <summary>Function key F1.</summary>
		F1           = 0x70,
		/// <summary>Function key F2.</summary>
		F2           = 0x71,
		/// <summary>Function key F3.</summary>
		F3           = 0x72,
		/// <summary>Function key F4.</summary>
		F4           = 0x73,
		/// <summary>Function key F5.</summary>
		F5           = 0x74,
		/// <summary>Function key F6.</summary>
		F6           = 0x75,
		/// <summary>Function key F7.</summary>
		F7           = 0x76,
		/// <summary>Function key F8.</summary>
		F8           = 0x77,
		/// <summary>Function key F9.</summary>
		F9           = 0x78,
		/// <summary>Function key F10.</summary>
		F10          = 0x79,
		/// <summary>Function key F11.</summary>
		F11          = 0x7A,
		/// <summary>Function key F12.</summary>
		F12          = 0x7B,
		/// <summary>,/&lt;</summary>
		Comma        = 0xBC,
		/// <summary>./&gt;</summary>
		Period       = 0xBE,
		/// <summary>/</summary>
		SlashFwd     = 0xBF,
		/// <summary>\</summary>
		SlashBack    = 0xDC,
		/// <summary>;/:</summary>
		Semicolon    = 0xBA,
		/// <summary>'/"</summary>
		Apostrophe   = 0xDE,
		/// <summary>[/{</summary>
		BracketOpen  = 0xDB,
		/// <summary>]/}</summary>
		BracketClose = 0xDD,
		/// <summary>-/_</summary>
		Minus        = 0xBD,
		/// <summary>=/+</summary>
		Equals       = 0xBB,
		/// <summary>`/~</summary>
		Backtick     = 0xc0,
		/// <summary>The Windows/Mac Command button on the left side of the keyboard.</summary>
		LCmd         = 0x5B,
		/// <summary>The Windows/Mac Command button on the right side of the keyboard.</summary>
		RCmd         = 0x5C,
		/// <summary>Numpad '*', NOT the same as number row '*'.</summary>
		Multiply     = 0x6A,
		/// <summary>Numpad '+', NOT the same as number row '+'.</summary>
		Add          = 0x6B,
		/// <summary>Numpad '-', NOT the same as number row '-'.</summary>
		Subtract     = 0x6D,
		/// <summary>Numpad '.', NOT the same as character '.'.</summary>
		Decimal      = 0x6E,
		/// <summary>Numpad '/', NOT the same as character '/'.</summary>
		Divide       = 0x6F,
		/// <summary>Maximum value for key codes.</summary>
		MAX          = 0xFF,
	}

	/// <summary>A settings flag that lets you describe the behavior of how
	/// StereoKit will refresh data about the world mesh, if applicable. This
	/// is used with `World.RefreshType`.</summary>
	public enum WorldRefresh {
		/// <summary>Refreshing occurs when the user leaves the area that was
		/// most recently scanned. This area is a sphere that is 0.5 of the
		/// World.RefreshRadius.</summary>
		Area,
		/// <summary>Refreshing happens at timer intervals. If an update
		/// doesn't happen in time, the next update will happen as soon as
		/// possible. The timer interval is configurable via
		/// `World.RefreshInterval`.</summary>
		Timer,
	}

	/// <summary>This describes what technology is being used to power StereoKit's
	/// XR backend.</summary>
	public enum BackendXRType {
		/// <summary>StereoKit is not using an XR backend of any sort. That means
		/// the application is flatscreen and has the simulator disabled.</summary>
		None,
		/// <summary>StereoKit is using the flatscreen XR simulator. Inputs are
		/// emulated, and some advanced XR functionality may not be
		/// available.</summary>
		Simulator,
		/// <summary>StereoKit is currently powered by OpenXR! This means we're
		/// running on a real XR device. Not all OpenXR runtimes provide
		/// the same functionality, but we will have access to more fun
		/// stuff :)</summary>
		OpenXR,
		/// <summary>StereoKit is running in a browser, and is using WebXR!</summary>
		WebXR,
	}

	/// <summary>This describes the platform that StereoKit is running on.</summary>
	public enum BackendPlatform {
		/// <summary>This is running as a Windows app using the Win32 APIs.</summary>
		Win32,
		/// <summary>This is running as a Windows app using the UWP APIs.</summary>
		Uwp,
		/// <summary>This is running as a Linux app.</summary>
		Linux,
		/// <summary>This is running as an Android app.</summary>
		Android,
		/// <summary>This is running in a browser.</summary>
		Web,
	}

	/// <summary>This describes the graphics API thatStereoKit is using for rendering.</summary>
	public enum BackendGraphics {
		/// <summary>An invalid default value. Right now, this may likely indicate a variety
		/// of OpenGL.</summary>
		None,
		/// <summary>DirectX's Direct3D11 is used for rendering!</summary>
		D3D11,
	}

	/// <summary>The log tool will write to the console with annotations for console
	/// colors, which helps with readability, but isn't always supported.
	/// These are the options available for configuring those colors.</summary>
	public enum LogColors {
		/// <summary>Use console coloring annotations.</summary>
		Ansi         = 0,
		/// <summary>Scrape out any color annotations, so logs are all completely
		/// plain text.</summary>
		None,
	}

}
