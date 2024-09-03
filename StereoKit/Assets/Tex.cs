using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>This is the texture asset class! This encapsulates 2D images,
	/// texture arrays, cubemaps, and rendertargets! It can load any image
	/// format that stb_image can, (jpg, png, tga, bmp, psd, gif, hdr, pic,
	/// ktx2) plus more later on, and you can also create textures
	/// procedurally.
	/// </summary>
	public class Tex : IAsset
	{
		#region Fields and Properties

		internal IntPtr                    _inst;
		private  List<Assets.CallbackData> _callbacks;

		/// <summary>Gets or sets the unique identifier of this asset resource!
		/// This can be helpful for debugging, managine your assets, or finding
		/// them later on!</summary>
		public string Id
		{
			get => Marshal.PtrToStringAnsi(NativeAPI.tex_get_id(_inst));
			set => NativeAPI.tex_set_id(_inst, value);
		}

		/// <summary> The width of the texture, in pixels. This will be a
		/// blocking call if AssetState is less than LoadedMeta. </summary>
		public int Width => NativeAPI.tex_get_width(_inst);
		/// <summary> The height of the texture, in pixels. This will be a
		/// blocking call if AssetState is less than LoadedMeta. </summary>
		public int Height => NativeAPI.tex_get_height(_inst);
		/// <summary>The number of mip-map levels this texture has. This will
		/// be 1 if the texture doesn't have mip mapping enabled. This will be
		/// a blocking call if AssetState is less than LoadedMeta.</summary>
		public int Mips => NativeAPI.tex_get_mips(_inst);
		/// <summary> The StereoKit format this texture was initialized with.
		/// This will be a blocking call if AssetState is less than LoadedMeta.
		/// </summary>
		public TexFormat Format => NativeAPI.tex_get_format(_inst);

		/// <summary> Textures are loaded asyncronously, so this tells you the
		/// current state of this texture! This also can tell if an error
		/// occured, and what type of error it may have been. </summary>
		public AssetState AssetState => NativeAPI.tex_asset_state(_inst);

		/// <summary> This will override the default fallback texutre that gets
		/// used before the Tex has finished loading. This is useful for
		/// textures with a specific purpose where the normal fallback texture
		/// would appear strange, such as a metal/rough map. </summary>
		public Tex FallbackOverride { set { NativeAPI.tex_set_fallback(_inst, value._inst); } }

		/// <summary>This event gets called</summary>
		public event Action<Tex> OnLoaded {
			add {
				if (_callbacks == null) _callbacks = new List<Assets.CallbackData>();

				AssetOnLoadCallback callback = (a, _) => { NativeAPI.tex_addref(a); value(new Tex(a)); };
				_callbacks.Add(new Assets.CallbackData { action = value, callback = callback });

				NativeAPI.tex_on_load(_inst, callback, IntPtr.Zero);
			}
			remove {
				if (_callbacks == null) throw new NullReferenceException();

				int i = _callbacks.FindIndex(d => (Action<Tex>)d.action == value);
				if (i<0) throw new KeyNotFoundException();

				NativeAPI.tex_on_load_remove(_inst, _callbacks[i].callback);
				_callbacks.RemoveAt(i);
			}
		}

		/// <summary>When looking at a UV texture coordinate on this texture,
		/// how do we handle values larger than 1, or less than zero? Do we
		/// Wrap to the other side? Clamp it between 0-1, or just keep
		/// Mirroring back and forth? Wrap is the default.</summary>
		public TexAddress AddressMode {
			get => NativeAPI.tex_get_address(_inst);
			set => NativeAPI.tex_set_address(_inst, value); }

		/// <summary>When sampling a texture that's stretched, or shrunk
		/// beyond its screen size, how do we handle figuring out which color
		/// to grab from the texture? Default is Linear.</summary>
		public TexSample SampleMode {
			get => NativeAPI.tex_get_sample(_inst);
			set => NativeAPI.tex_set_sample(_inst, value); }

		/// <summary>When SampleMode is set to Anisotropic, this is the number
		/// of samples the GPU takes to figure out the correct color. Default
		/// is 4, and 16 is pretty high.</summary>
		public int Anisoptropy {
			get => NativeAPI.tex_get_anisotropy(_inst);
			set => NativeAPI.tex_set_anisotropy(_inst, value); }

		/// <summary>ONLY valid for cubemap textures! This will calculate a
		/// spherical harmonics representation of the cubemap for use with
		/// StereoKit's lighting. First call may take a frame or two of time,
		/// but subsequent calls will pull from a cached value.</summary>
		public SphericalHarmonics CubemapLighting => NativeAPI.tex_get_cubemap_lighting(_inst);

		/// <summary>This allows you to attach or retreive a z/depth buffer
		/// from a rendertarget texture. This texture _must_ be a rendertarget
		/// to set this, and the zbuffer texture _must_ be a depth format (or
		/// null). For no-rendertarget textures, this will always be null.
		/// </summary>
		public Tex ZBuffer {
			get {
				IntPtr result = NativeAPI.tex_get_zbuffer(_inst);
				return result == IntPtr.Zero
					? null
					: new Tex(result);
			}
			set => NativeAPI.tex_set_zbuffer(_inst, value._inst == IntPtr.Zero ? IntPtr.Zero : value._inst);
		}
		#endregion

		#region Constructors

		/// <summary>Sets up an empty texture container! Fill it with data 
		/// using SetColors next! Creates a default unique asset Id.</summary>
		/// <param name="textureType">What type of texture is it? Just a 2D 
		/// Image? A Cubemap? Should it have mip-maps?</param>
		/// <param name="textureFormat">What information is the texture 
		/// composed of? 32 bit colors, 64 bit colors, etc.</param>
		public Tex(TexType textureType = TexType.Image, TexFormat textureFormat = TexFormat.Rgba32)
		{
			_inst = NativeAPI.tex_create(textureType, textureFormat);
		}
		internal Tex(IntPtr tex)
		{
			_inst = tex;
			if (_inst == IntPtr.Zero)
				Log.Err("Received an empty texture!");
		}
		/// <summary>Release reference to the StereoKit asset.</summary>
		~Tex()
		{
			if (_inst != IntPtr.Zero)
			{
				if (_callbacks != null)
				{
					foreach (var c in _callbacks)
					{
						NativeAPI.tex_on_load_remove(_inst, c.callback);
					}
					_callbacks = null;
				}
				NativeAPI.assets_releaseref_threadsafe(_inst);
			}
		}

		#endregion

		#region Methods
		/// <summary>Set the texture's pixels using a pointer to a chunk of
		/// memory! This is great if you're pulling in some color data from
		/// native code, and don't want to pay the cost of trying to marshal
		/// that data around.</summary>
		/// <param name="width">Width in pixels of the texture. Powers of two
		/// are generally best!</param>
		/// <param name="height">Height in pixels of the texture. Powers of
		/// two are generally best!</param>
		/// <param name="data">A pointer to a chunk of memory containing color
		/// data! Should be  width*height*size_of_texture_format bytes large.
		/// Color data should definitely match the format provided when 
		/// constructing the texture!</param>
		public void SetColors(int width, int height, IntPtr data)
			=> NativeAPI.tex_set_colors(_inst, width, height, data);
		/// <summary>Set the texture's pixels using a color array! This
		/// function should only be called on textures with a format of
		/// Rgba32 or Rgba32Linear. You can call this as many times as you'd
		/// like, even with different widths and heights. Calling this
		/// multiple times will mark it as dynamic on the graphics card.
		/// Calling this function can also result in building mip-maps, which
		/// has a non-zero cost: use TexType.ImageNomips when creating the
		/// Tex to avoid this.</summary>
		/// <param name="width">Width in pixels of the texture. Powers of two
		/// are generally best!</param>
		/// <param name="height">Height in pixels of the texture. Powers of
		/// two are generally best!</param>
		/// <param name="data">An array of 32 bit colors, should be a length
		/// of `width*height`.</param>
		public void SetColors(int width, int height, in Color32[] data)
		{
			TexFormat format = Format;
			if (format != TexFormat.Rgba32 && format != TexFormat.Rgba32Linear)
			{
				Log.Err($"Can't set a {format} format texture from Color32 data!");
				return;
			}
			NativeAPI.tex_set_colors(_inst, width, height, data);
		}
		/// <summary>Set the texture's pixels using a color array! This
		/// function should only be called on textures with a format of Rgba128.
		/// You can call this as many times as you'd like, even with different
		/// widths and heights. Calling this multiple times will mark it as
		/// dynamic on the graphics card. Calling this function can also
		/// result in building mip-maps, which has a non-zero cost: use
		/// TexType.ImageNomips when creating the Tex to avoid this.</summary>
		/// <param name="width">Width in pixels of the texture. Powers of two
		/// are generally best!</param>
		/// <param name="height">Height in pixels of the texture. Powers of
		/// two are generally best!</param>
		/// <param name="data">An array of 128 bit colors, should be a length
		/// of `width*height`.</param>
		public void SetColors(int width, int height, in Color[] data)
		{
			if (Format != TexFormat.Rgba128)
			{
				Log.Err($"Can't set a {Format} format texture from Color data!");
				return;
			}
			NativeAPI.tex_set_colors(_inst, width, height, data);
		}
		/// <summary>Set the texture's pixels using a scalar array! This
		/// function should only be called on textures with a format of R8.
		/// You can call this as many times as you'd like, even with different
		/// widths and heights. Calling this multiple times will mark it as
		/// dynamic on the graphics card. Calling this function can also
		/// result in building mip-maps, which has a non-zero cost: use
		/// TexType.ImageNomips when creating the Tex to avoid this.</summary>
		/// <param name="width">Width in pixels of the texture. Powers of two
		/// are generally best!</param>
		/// <param name="height">Height in pixels of the texture. Powers of
		/// two are generally best!</param>
		/// <param name="data">An array of 8 bit values, should be a length
		/// of `width*height`.</param>
		public void SetColors(int width, int height, in byte[] data)
		{
			NativeAPI.tex_set_colors(_inst, width, height, data);
		}
		/// <summary>Set the texture's pixels using a scalar array! This
		/// function should only be called on textures with a format of R16.
		/// You can call this as many times as you'd like, even with different
		/// widths and heights. Calling this multiple times will mark it as
		/// dynamic on the graphics card. Calling this function can also
		/// result in building mip-maps, which has a non-zero cost: use
		/// TexType.ImageNomips when creating the Tex to avoid this.</summary>
		/// <param name="width">Width in pixels of the texture. Powers of two
		/// are generally best!</param>
		/// <param name="height">Height in pixels of the texture. Powers of
		/// two are generally best!</param>
		/// <param name="data">An array of 16 bit values, should be a length
		/// of `width*height`.</param>
		public void SetColors(int width, int height, in ushort[] data)
		{
			TexFormat format = Format;
			if (format != TexFormat.R16 && format != TexFormat.Rgba64)
			{
				Log.Err($"Can't set a {format} format texture from ushort data!");
				return;
			}
			NativeAPI.tex_set_colors(_inst, width, height, data);
		}
		/// <summary>Set the texture's pixels using a scalar array! This
		/// function should only be called on textures with a format of R32.
		/// You can call this as many times as you'd like, even with different
		/// widths and heights. Calling this multiple times will mark it as
		/// dynamic on the graphics card. Calling this function can also
		/// result in building mip-maps, which has a non-zero cost: use
		/// TexType.ImageNomips when creating the Tex to avoid this.</summary>
		/// <param name="width">Width in pixels of the texture. Powers of two
		/// are generally best!</param>
		/// <param name="height">Height in pixels of the texture. Powers of
		/// two are generally best!</param>
		/// <param name="data">An array of 32 bit values, should be a length
		/// of `width*height`.</param>
		public void SetColors(int width, int height, in float[] data)
		{
			if (Format != TexFormat.R32)
			{
				Log.Err($"Can't set a {Format} format texture from Color data!");
				return;
			}
			NativeAPI.tex_set_colors(_inst, width, height, data);
		}

		/// <summary>Loads an image file stored in memory directly into
		/// the created texture! Supported formats are: jpg, png, tga,
		/// bmp, psd, gif, hdr, pic, ktx2. This method introduces a blocking
		/// boolean parameter, which allows you to specify whether this
		/// method blocks until the image fully loads! The default case
		/// is to have it as part of the asynchronous asset pipeline, in
		/// which the Asset Id will be the same as the filename.</summary>
		/// <param name="imageFileData">The binary data of an image file,
		/// this is NOT a raw RGB color array!</param>
		/// <param name="sRGBData">Is this image color data in sRGB format,
		/// or is it normal/metal/rough/data that's not for direct display?
		/// sRGB colors get converted to linear color space on the graphics
		/// card, so getting this right can have a big impact on visuals.
		/// </param>
		/// <param name="blocking">Will this method wait for the image 
		/// to load. By default, we try to load it asynchronously.</param>
		/// <param name="priority">The priority sort order for this asset in
		/// the async loading system. Lower values mean loading sooner.</param>
		public void SetMemory(in byte[] imageFileData, bool sRGBData = true, bool blocking = false, int priority = 10)
		{
			NativeAPI.tex_set_mem(_inst, imageFileData, (UIntPtr)imageFileData.Length, sRGBData, blocking, priority);
		}

		/// <summary>This function is dependent on the graphics backend! It
		/// will take a texture resource for the current graphics backend (D3D
		/// or GL) and wrap it in a StereoKit texture for use within StereoKit.
		/// This is a bit of an advanced feature.</summary>
		/// <param name="nativeTexture">For D3D, this should be an
		/// ID3D11Texture2D*, and for GL, this should be a uint32_t from a
		/// glGenTexture call, coerced into the IntPtr.</param>
		/// <param name="type">The image flags that tell SK how to treat the
		/// texture, this should match up with the settings the texture was
		/// originally created with. If SK can figure the appropriate settings,
		/// it _may_ override the value provided here.</param>
		/// <param name="native_fmt">The texture's format using the graphics
		/// backend's value, not SK's. This should match up with the settings
		/// the texture was originally created with. If SK can figure the
		/// appropriate settings, it _may_ override the value provided here.
		/// </param>
		/// <param name="width">Width of the texture. This should match up with
		/// the settings the texture was originally created with. If SK can
		/// figure the appropriate settings, it _may_ override the value
		/// provided here.</param>
		/// <param name="height">Height of the texture. This should match up
		/// with the settings the texture was originally created with. If SK
		/// can figure the appropriate settings, it _may_ override the value
		/// provided here.</param>
		/// <param name="surface_count">Texture array surface count. This
		/// should match up with the settings the texture was originally
		/// created with. If SK can figure the appropriate settings, it _may_
		/// override the value provided here.</param>
		/// <param name="owned">Should ownership of this texture resource be
		/// passed on to StereoKit? If so, StereoKit may delete it when it's
		/// finished with it. If this is not desired, pass in false.</param>
		public void SetNativeSurface(IntPtr nativeTexture, TexType type=TexType.Image, long native_fmt=0, int width=0, int height=0, int surface_count=1, bool owned=true)
			=> NativeAPI.tex_set_surface(_inst, nativeTexture, type, native_fmt, width, height, surface_count, 1, 1, owned);

		/// <summary>This will return the texture's native resource for use
		/// with external libraries. For D3D, this will be an ID3D11Texture2D*,
		/// and for GL, this will be a uint32_t from a glGenTexture call,
		/// coerced into the IntPtr. This call will block execution until the
		/// texture is loaded, if it is not already.</summary>
		/// <returns>For D3D, this will be an ID3D11Texture2D*, and for GL,
		/// this will be a uint32_t from a glGenTexture call, coerced into the
		/// IntPtr.</returns>
		public IntPtr GetNativeSurface()
			=> NativeAPI.tex_get_surface(_inst);

		/// <summary>Retrieve the color data of the texture from the GPU. This
		/// can be a very slow operation, so use it cautiously.</summary>
		/// <typeparam name="T">This should be a struct or basic type used to
		/// represent your color/pixel data. Structs should use
		/// `[StructLayout(LayoutKind.Sequential)]`.</typeparam>
		/// <param name="mipLevel">Retrieves the color data for a specific
		/// mip-mapping level. This function will log a fail and return a black
		/// array if an invalid mip-level is provided.</param>
		/// <param name="structPerPixel">The number of `T` that fit in a single
		/// pixel. For example, if your texture format is RGBA128, and your T
		/// is float, this value would be 4.</param>
		/// <returns>The texture's color values in an array sized
		/// Width*Height*structPerPixel.</returns>
		public T[] GetColorData<T>(int mipLevel = 0, int structPerPixel = 1) where T:struct
		{
			T[] result = null;
			GetColorData(ref result, mipLevel, structPerPixel);
			return result;
		}

		/// <summary>Retrieve the color data of the texture from the GPU. This
		/// can be a very slow operation, so use it cautiously.</summary>
		/// <typeparam name="T">This should be a struct or basic type used to
		/// represent your color/pixel data. Structs should use
		/// `[StructLayout(LayoutKind.Sequential)]`.</typeparam>
		/// <param name="colorData">An array of colors that will be filled out
		/// with the texture's data. It can be null, or an incorrect size. If
		/// so, it will be reallocated to the correct size.</param>
		/// <param name="mipLevel">Retrieves the color data for a specific
		/// mip-mapping level. This function will log a fail and return a black
		/// array if an invalid mip-level is provided.</param>
		/// <param name="structPerPixel">The number of `T` that fit in a single
		/// pixel. For example, if your texture format is RGBA128, and your T
		/// is float, this value would be 4.</param>
		/// <exception cref="ArgumentException">structPerPixel must be larger
		/// than 0</exception>
		public void GetColorData<T>(ref T[] colorData, int mipLevel = 0, int structPerPixel = 1) where T:struct
		{
			int structSize  = Marshal.SizeOf<T>();
			int pixelSize   = structPerPixel * structSize;

			if (structPerPixel <= 0)
				throw new ArgumentException("structPerPixel must be larger than 0");

			int width      = Width  >> mipLevel;
			int height     = Height >> mipLevel;
			int dataLength = width * height * structPerPixel;
			if (colorData == null || colorData.Length != dataLength) colorData = new T[dataLength];

			GCHandle  pinnedArray = GCHandle.Alloc(colorData, GCHandleType.Pinned);
			IntPtr    pointer     = pinnedArray.AddrOfPinnedObject();
			NativeAPI.tex_get_data(_inst, pointer, (UIntPtr)(width * height * pixelSize), mipLevel);
			pinnedArray.Free();
		}

		/// <summary>Set the texture's size without providing any color data.
		/// In most cases, you should probably just call SetColors instead,
		/// but this can be useful if you're adding color data some other
		/// way, such as when blitting or rendering to it.</summary>
		/// <param name="width">Width in pixels of the texture. Powers of two
		/// are generally best!</param>
		/// <param name="height">Height in pixels of the texture. Powers of
		/// two are generally best!</param>
		public void SetSize(int width, int height)
			=> NativeAPI.tex_set_colors(_inst, width, height, IntPtr.Zero);

		/// <summary>Only applicable if this texture is a rendertarget!
		/// This creates and attaches a zbuffer surface to the texture for
		/// use when rendering to it.</summary>
		/// <param name="depthFormat">The format of the depth texture, must
		/// be a depth format type!</param>
		/// <returns>A new Tex asset with the specified depth.</returns>
		public void AddZBuffer(TexFormat depthFormat)
			=> NativeAPI.tex_add_zbuffer(_inst, depthFormat);
		#endregion

		#region Static Methods

		/// <summary>This is the texture that all Tex objects will fall back to
		/// by default if they are still loading. Assigning a texture here that
		/// isn't fully loaded will cause the app to block until it is loaded.
		/// </summary>
		/// <param name="loadingTexture">Any _valid_ texture here is fine.
		/// Preferably loaded already, but doesn't have to be.</param>
		public static void SetLoadingFallback(Tex loadingTexture)
			=> NativeAPI.tex_set_loading_fallback(loadingTexture._inst);

		/// <summary>This is the texture that all Tex objects with errors will
		/// fall back to. Assigning a texture here that isn't fully loaded will
		/// cause the app to block until it is loaded. </summary>
		/// <param name="errorTexture">Any _valid_ texture here is fine.
		/// Preferably loaded already, but doesn't have to be.</param>
		public static void SetErrorFallback(Tex errorTexture)
			=> NativeAPI.tex_set_error_fallback(errorTexture._inst);

		/// <summary>Finds a texture that matches the given Id! Check out the
		/// DefaultIds static class for some built-in ids.</summary>
		/// <param name="id">Id of the texture asset.</param>
		/// <returns>A Tex asset with the given id, or null if none is found.
		/// </returns>
		public static Tex Find(string id)
		{
			IntPtr tex = NativeAPI.tex_find(id);
			return tex == IntPtr.Zero ? null : new Tex(tex);
		}

		/// <summary>Creates a cubemap texture from a single equirectangular 
		/// image! You know, the ones that look like an unwrapped globe with
		/// the poles all stretched out. It uses some fancy shaders and
		/// texture blitting to create 6 faces from the equirectangular 
		/// image.</summary>
		/// <param name="equirectangularCubemap">Filename of the
		/// equirectangular image.</param>
		/// <param name="sRGBData">Is this image color data in sRGB format,
		/// or is it normal/metal/rough/data that's not for direct display?
		/// sRGB colors get converted to linear color space on the graphics
		/// card, so getting this right can have a big impact on visuals.
		/// </param>
		/// <param name="loadPriority">The priority sort order for this asset
		/// in the async loading system. Lower values mean loading sooner.
		/// </param>
		/// <returns>A Cubemap texture asset!</returns>
		public static Tex FromCubemapEquirectangular(string equirectangularCubemap, bool sRGBData = true, int loadPriority = 10)
		{
			IntPtr tex = NativeAPI.tex_create_cubemap_file(NativeHelper.ToUtf8(equirectangularCubemap), sRGBData, IntPtr.Zero, loadPriority);
			return tex == IntPtr.Zero ? null : new Tex(tex);
		}

		/// <summary>Creates a cubemap texture from a single equirectangular
		/// image! You know, the ones that look like an unwrapped globe with
		/// the poles all stretched out. It uses some fancy shaders and
		/// texture blitting to create 6 faces from the equirectangular image.
		/// </summary>
		/// <param name="equirectangularCubemap">Filename of the
		/// equirectangular image.</param>
		/// <param name="lightingInfo">An out value that represents the
		/// lighting information scraped from the cubemap! This can then be
		/// passed to `Renderer.SkyLight`.</param>
		/// <param name="sRGBData">Is this image color data in sRGB format,
		/// or is it normal/metal/rough/data that's not for direct display?
		/// sRGB colors get converted to linear color space on the graphics
		/// card, so getting this right can have a big impact on visuals.
		/// </param>
		/// <param name="loadPriority">The priority sort order for this asset
		/// in the async loading system. Lower values mean loading sooner.
		/// </param>
		/// <returns>A Cubemap texture asset!</returns>
		public static Tex FromCubemapEquirectangular(string equirectangularCubemap, out SphericalHarmonics lightingInfo, bool sRGBData = true, int loadPriority = 10)
		{
			IntPtr tex = NativeAPI.tex_create_cubemap_file(NativeHelper.ToUtf8(equirectangularCubemap), sRGBData, out lightingInfo, loadPriority);
			return tex == IntPtr.Zero ? null : new Tex(tex);
		}

		/// <summary>Loads an image file directly into a texture! Supported
		/// formats are: jpg, png, tga, bmp, psd, gif, hdr, pic, ktx2. Asset Id
		/// will be the same as the filename.</summary>
		/// <param name="file">An absolute filename, or a filename relative
		/// to the assets folder. Supports jpg, png, tga, bmp, psd, gif, hdr,
		/// pic, ktx2.</param>
		/// <param name="sRGBData">Is this image color data in sRGB format,
		/// or is it normal/metal/rough/data that's not for direct display?
		/// sRGB colors get converted to linear color space on the graphics
		/// card, so getting this right can have a big impact on visuals.
		/// </param>
		/// <param name="loadPriority">The priority sort order for this asset
		/// in the async loading system. Lower values mean loading sooner.
		/// </param>
		/// <returns>A Tex asset from the given file, or null if it failed to
		/// load.</returns>
		public static Tex FromFile(string file, bool sRGBData = true, int loadPriority = 10)
		{
			IntPtr inst = NativeAPI.tex_create_file(NativeHelper.ToUtf8(file), sRGBData, 10);
			return inst == IntPtr.Zero ? null : new Tex(inst);
		}

		/// <summary>Loads an array of image files directly into a single
		/// array texture! Array textures are often useful for shader
		/// effects, layering, material merging, weird stuff, and will
		/// generally need a specific shader to support it. Supported formats
		/// are: jpg, png, tga, bmp, psd, gif, hdr, pic, ktx2. Asset Id will be
		/// the hash of all the filenames merged consecutively.</summary>
		/// <param name="files">Absolute filenames, or a filenames relative
		/// to the assets folder. Supports jpg, png, tga, bmp, psd, gif, hdr,
		/// pic, ktx2.</param>
		/// <param name="sRGBData">Is this image color data in sRGB format,
		/// or is it normal/metal/rough/data that's not for direct display?
		/// sRGB colors get converted to linear color space on the graphics
		/// card, so getting this right can have a big impact on visuals.
		/// </param>
		/// <param name="priority">The priority sort order for this asset in
		/// the async loading system. Lower values mean loading sooner.</param>
		/// <returns>A Tex asset from the given files, or null if it failed
		/// to load.</returns>
		public static Tex FromFiles(string[] files, bool sRGBData = true, int priority = 10)
		{
			IntPtr inst = NativeAPI.tex_create_file_arr(files, files.Length, sRGBData, priority);
			return inst == IntPtr.Zero ? null : new Tex(inst);
		}

		/// <summary>Loads an image file stored in memory directly into a
		/// texture! Supported formats are: jpg, png, tga, bmp, psd, gif,
		/// hdr, pic, ktx2. Asset Id will be the same as the filename.
		/// </summary>
		/// <param name="imageFileData">The binary data of an image file,
		/// this is NOT a raw RGB color array!</param>
		/// <param name="sRGBData">Is this image color data in sRGB format,
		/// or is it normal/metal/rough/data that's not for direct display?
		/// sRGB colors get converted to linear color space on the graphics
		/// card, so getting this right can have a big impact on visuals.
		/// </param>
		/// <param name="priority">The priority sort order for this asset in
		/// the async loading system. Lower values mean loading sooner.</param>
		/// <returns>A Tex asset from the given file, or null if it failed to
		/// load.</returns>
		public static Tex FromMemory(in byte[] imageFileData, bool sRGBData = true, int priority = 10)
		{
			IntPtr inst = NativeAPI.tex_create_mem(imageFileData, (UIntPtr)imageFileData.Length, sRGBData, priority);
			return inst == IntPtr.Zero ? null : new Tex(inst);
		}

		/// <summary>Creates a texture and sets the texture's pixels using a
		/// color array! This will be an image of type `TexType.Image`, and
		/// a format of `TexFormat.Rgba32` or `TexFormat.Rgba32Linear`
		/// depending on the value of the sRGBData parameter.</summary>
		/// <param name="colors">An array of 32 bit colors, should be a
		/// length of `width*height`.</param>
		/// <param name="width">Width in pixels of the texture. Powers of two
		/// are generally best!</param>
		/// <param name="height">Height in pixels of the texture. Powers of
		/// two are generally best!</param>
		/// <param name="sRGBData">Is this image color data in sRGB format,
		/// or is it normal/metal/rough/data that's not for direct display?
		/// sRGB colors get converted to linear color space on the graphics
		/// card, so getting this right can have a big impact on visuals.
		/// </param>
		/// <returns>A Tex asset with TexType.Image and TexFormat.Rgba32 from
		/// the given array of colors.</returns>
		public static Tex FromColors(in Color32[] colors, int width, int height, bool sRGBData = true)
		{
			if (colors.Length < width*height) throw new ArgumentException("colors.Length < width*height");

			IntPtr inst = NativeAPI.tex_create_color32(colors, width, height, sRGBData);
			return inst == IntPtr.Zero ? null : new Tex(inst);
		}

		/// <summary>Creates a texture and sets the texture's pixels using a
		/// color array! Color values are converted to 32 bit colors, so this
		/// means a memory allocation and conversion. Prefer the Color32
		/// overload for performance, or create an empty Texture and use
		/// SetColors for more flexibility. This will be an image of type
		/// `TexType.Image`, and a format of `TexFormat.Rgba32` or
		/// `TexFormat.Rgba32Linear` depending on the value of the sRGBData
		/// parameter.</summary>
		/// <param name="colors">An array of 128 bit colors, should be a
		/// length of `width*height`.</param>
		/// <param name="width">Width in pixels of the texture. Powers of two
		/// are generally best!</param>
		/// <param name="height">Height in pixels of the texture. Powers of
		/// two are generally best!</param>
		/// <param name="sRGBData">Is this image color data in sRGB format,
		/// or is it normal/metal/rough/data that's not for direct display?
		/// sRGB colors get converted to linear color space on the graphics
		/// card, so getting this right can have a big impact on visuals.
		/// </param>
		/// <returns>A Tex asset with TexType.Image and TexFormat.Rgba32 from
		/// the given array of colors.</returns>
		public static Tex FromColors(in Color[] colors, int width, int height, bool sRGBData = true)
		{
			if (colors.Length < width*height) throw new ArgumentException("colors.Length < width*height");

			IntPtr inst = NativeAPI.tex_create_color128(colors, width, height, sRGBData);
			return inst == IntPtr.Zero ? null : new Tex(inst);
		}


		/// <summary>Creates a cubemap texture from 6 different image files!
		/// If you have a single equirectangular image, use
		/// Tex.FromEquirectangular  instead. Asset Id will be the first
		/// filename.</summary>
		/// <param name="cubeFaceFiles_xxyyzz">6 image filenames, in order of
		/// +X, -X, +Y, -Y, +Z, -Z.</param>
		/// <param name="sRGBData">Is this image color data in sRGB format,
		/// or is it normal/metal/rough/data that's not for direct display?
		/// sRGB colors get converted to linear color space on the graphics
		/// card, so getting this right can have a big impact on visuals.
		/// </param>
		/// <param name="priority">The priority sort order for this asset in
		/// the async loading system. Lower values mean loading sooner.</param>
		/// <returns>A Tex asset from the given files, or null if any failed 
		/// to load.</returns>
		public static Tex FromCubemapFile(string[] cubeFaceFiles_xxyyzz, bool sRGBData = true, int priority = 10)
		{
			if (cubeFaceFiles_xxyyzz.Length != 6)
				Log.Err("To create a cubemap, you must have exactly 6 images!");
			IntPtr inst = NativeAPI.tex_create_cubemap_files(cubeFaceFiles_xxyyzz, sRGBData, IntPtr.Zero, priority);
			return inst == IntPtr.Zero ? null : new Tex(inst);
		}

		/// <summary>Creates a cubemap texture from 6 different image files!
		/// If you have a single equirectangular image, use
		/// Tex.FromEquirectangular instead. Asset Id will be the first
		/// filename.</summary>
		/// <param name="cubeFaceFiles_xxyyzz">6 image filenames, in order of
		/// +X, -X, +Y, -Y, +Z, -Z.</param>
		/// <param name="lightingInfo">An out value that represents the
		/// lighting information scraped from the cubemap! This can then be
		/// passed to `Renderer.SkyLight`.</param>
		/// <param name="sRGBData">Is this image color data in sRGB format,
		/// or is it normal/metal/rough/data that's not for direct display?
		/// sRGB colors get converted to linear color space on the graphics
		/// card, so getting this right can have a big impact on visuals.
		/// </param>
		/// <param name="priority">The priority sort order for this asset in
		/// the async loading system. Lower values mean loading sooner.</param>
		/// <returns>A Tex asset from the given files, or null if any failed 
		/// to load.</returns>
		public static Tex FromCubemapFile(string[] cubeFaceFiles_xxyyzz, out SphericalHarmonics lightingInfo, bool sRGBData = true, int priority = 10)
		{
			if (cubeFaceFiles_xxyyzz.Length != 6)
				Log.Err("To create a cubemap, you must have exactly 6 images!");
			IntPtr inst = NativeAPI.tex_create_cubemap_files(cubeFaceFiles_xxyyzz, sRGBData, out lightingInfo, priority);
			return inst == IntPtr.Zero ? null : new Tex(inst);
		}

		/// <summary>This will assemble a texture ready for rendering to! It
		/// creates a render target texture with no mip maps and a depth buffer
		/// attached.</summary>
		/// <param name="width">Width in pixels.</param>
		/// <param name="height">Height in pixels</param>
		/// <param name="multisample">Multisample level, or MSAA. This should
		/// be 1, 2, 4, 8, or 16. The results will have moother edges with
		/// higher values, but will cost more RAM and time to render. Note that
		/// GL platforms cannot trivially draw a multisample > 1 texture in a
		/// shader.</param>
		/// <param name="colorFormat">The format of the color surface.</param>
		/// <param name="depthFormat">The format of the depth buffer. If this
		/// is None, no depth buffer will be attached to this rendertarget.</param>
		/// <returns>Returns a texture set up as a rendertarget.</returns>
		public static Tex RenderTarget(int width, int height, int multisample = 1, TexFormat colorFormat = TexFormat.Rgba32, TexFormat depthFormat = TexFormat.Depth16)
		{
			IntPtr tex =  NativeAPI.tex_create_rendertarget(width, height, multisample, colorFormat, depthFormat);
			return tex == IntPtr.Zero ? null : new Tex(tex);
		}

		/// <summary>This generates a solid color texture of the given
		/// dimensions. Can be quite nice for creating placeholder textures!
		/// Make sure to match linear/gamma colors with the correct format.
		/// </summary>
		/// <param name="color">The color to use for the texture. This is
		/// interpreted slightly differently based on what TexFormat gets used.
		/// </param>
		/// <param name="width">Width of the final texture, in pixels.</param>
		/// <param name="height">Height of the final texture, in pixels.
		/// </param>
		/// <param name="type">Not all types here are applicable, but
		/// TexType.Image or TexType.ImageNomips are good options here.</param>
		/// <param name="format">Not all formats are supported, but this does
		/// support a decent range. The provided color is interpreted slightly
		/// different depending on this format.</param>
		/// <returns>A solid color image of width x height pixels.</returns>
		public static Tex GenColor(Color color, int width, int height, TexType type = TexType.Image, TexFormat format = TexFormat.Rgba32)
		{
			IntPtr tex = NativeAPI.tex_gen_color(color, width, height, type, format);
			return tex == IntPtr.Zero ? null : new Tex(tex);
		}

		/// <summary>Generates a 'radial' gradient that works well for
		/// particles, blob shadows, glows, or various other things. The
		/// roundness can be used to change the shape from round, '1', to
		/// star-like, '0'. Default color is transparent white to opaque white,
		/// but this can be configured by providing a Gradient of your own.
		/// </summary>
		/// <param name="width">Width of the desired texture, in pixels.
		/// </param>
		/// <param name="height">Width of the desired texture, in pixels.
		/// </param>
		/// <param name="roundness0to1">Where 0 is a cross, or star-like shape,
		/// and 1 is a circle. This is clamped to a minimum of 0.00001, but
		/// values above 1 are still valid, and will just make the shape a
		/// square near infinity.</param>
		/// <param name="gradientLinear">A color gradient that starts with the
		/// background/outside at 0, and progresses to the center at 1.</param>
		/// <returns>A texture object containing an RGBA linear texture.
		/// </returns>
		public static Tex GenParticle(int width, int height, float roundness0to1= 1, Gradient gradientLinear = null)
		{
			IntPtr tex = NativeAPI.tex_gen_particle(width, height, roundness0to1, gradientLinear==null?IntPtr.Zero:gradientLinear._inst);
			return tex == IntPtr.Zero ? null : new Tex(tex);
		}

		/// <summary>Generates a cubemap texture from a gradient and a
		/// direction! These are entirely suitable for skyboxes, which you
		/// can set via Renderer.SkyTex.</summary>
		/// <param name="gradient">A color gradient the generator will sample
		/// from! This looks at the 0-1 range of the gradient.</param>
		/// <param name="gradientDirection">This vector points to where the
		/// 'top' of the color gradient will go. Conversely, the 'bottom' of
		/// the gradient will be opposite, and it'll blend along that axis.
		/// </param>
		/// <param name="resolution">The square size in pixels of each cubemap
		/// face! This generally doesn't need to be large, unless you have a
		/// really complicated gradient.</param>
		/// <returns>A procedurally generated cubemap texture!</returns>
		public static Tex GenCubemap(Gradient gradient, Vec3 gradientDirection, int resolution = 16)
		{
			IntPtr tex = NativeAPI.tex_gen_cubemap(gradient._inst, gradientDirection, resolution, IntPtr.Zero);
			return tex == IntPtr.Zero ? null : new Tex(tex);
		}

		/// <summary>Generates a cubemap texture from a gradient and a
		/// direction! These are entirely suitable for skyboxes, which you
		/// can set via `Renderer.SkyTex`.</summary>
		/// <param name="gradient">A color gradient the generator will sample
		/// from! This looks at the 0-1 range of the gradient.</param>
		/// <param name="lightingInfo">An out value that represents the
		/// lighting information scraped from the cubemap! This can then be
		/// passed to `Renderer.SkyLight`.</param>
		/// <param name="gradientDirection">This vector points to where the
		/// 'top' of the color gradient will go. Conversely, the 'bottom' of
		/// the gradient will be opposite, and it'll blend along that axis.
		/// </param>
		/// <param name="resolution">The square size in pixels of each cubemap
		/// face! This generally doesn't need to be large, unless you have a
		/// really complicated gradient.</param>
		/// <returns>A procedurally generated cubemap texture!</returns>
		public static Tex GenCubemap(Gradient gradient, out SphericalHarmonics lightingInfo, Vec3 gradientDirection, int resolution = 16)
		{
			IntPtr tex = NativeAPI.tex_gen_cubemap(gradient._inst, gradientDirection, resolution, out lightingInfo);
			return tex == IntPtr.Zero ? null : new Tex(tex);
		}

		/// <summary>Creates a cubemap from SphericalHarmonics lookups! These
		/// are entirely suitable for skyboxes, which you can set via
		/// `Renderer.SkyTex`.</summary>
		/// <param name="lighting">Lighting information stored in a
		/// SphericalHarmonics.</param>
		/// <param name="resolution">The square size in pixels of each cubemap
		/// face! This generally doesn't need to be large, as
		/// SphericalHarmonics typically contain pretty low frequency
		/// information.</param>
		/// <param name="lightSpotSizePct">The size of the glowing spot added
		/// in the primary light direction. You can kinda think of the unit
		/// as a percentage of the cubemap face's size, but it's technically
		/// a Chebyshev distance from the light's point on a 2m cube.</param>
		/// <param name="lightSpotIntensity">The glowing spot's color is the
		/// primary light direction's color, but multiplied by this value.
		/// Since this method generates a 128bpp texture, this is not clamped
		/// between 0-1, so feel free to go nuts here! Remember that 
		/// reflections will often cut down some reflection intensity.</param>
		/// <returns>A procedurally generated cubemap texture!</returns>
		public static Tex GenCubemap(in SphericalHarmonics lighting, int resolution = 16, float lightSpotSizePct = 0, float lightSpotIntensity = 6)
		{
			IntPtr tex = NativeAPI.tex_gen_cubemap_sh(lighting, resolution, lightSpotSizePct, lightSpotIntensity);
			return tex == IntPtr.Zero ? null : new Tex(tex);
		}
		#endregion

		#region Static Properties
		/// <inheritdoc cref="Default.Tex" />
		public static Tex White => Default.Tex;
		/// <inheritdoc cref="Default.TexBlack" />
		public static Tex Black => Default.TexBlack;
		/// <inheritdoc cref="Default.TexFlat" />
		public static Tex Flat => Default.TexFlat;
		/// <inheritdoc cref="Default.TexGray" />
		public static Tex Gray => Default.TexGray;
		/// <inheritdoc cref="Default.TexRough" />
		public static Tex Rough => Default.TexRough;
		/// <inheritdoc cref="Default.TexDevTex" />
		public static Tex DevTex => Default.TexDevTex;
		/// <inheritdoc cref="Default.TexError" />
		public static Tex Error => Default.TexError;
		#endregion
	}
}
