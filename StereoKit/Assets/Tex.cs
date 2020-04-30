using System;
using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>This is the texture asset class! This encapsulates 2D images,
	/// texture arrays, cubemaps, and rendertargets! It can load any image
	/// format that stb_image can, (jpg, png, tga, bmp, psd, gif, hdr, pic)
	/// plus more later on, and you can also create textures procedurally.</summary>
	public class Tex
	{
		#region Fields and Properties

		internal IntPtr _inst;

		/// <summary>Allows you to set the Id of the texture to a specific 
		/// Id.</summary>
		public string Id { set => NativeAPI.tex_set_id(_inst, value); }

		/// <summary> The width of the texture, in pixels. </summary>
		public int Width => NativeAPI.tex_get_width(_inst);
		/// <summary> The height of the texture, in pixels. </summary>
		public int Height => NativeAPI.tex_get_height(_inst);
		/// <summary> The StereoKit format this texture was initialized with.
		/// </summary>
		public TexFormat Format => NativeAPI.tex_get_format(_inst);

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
		~Tex()
		{
			if (_inst != IntPtr.Zero)
				NativeAPI.tex_release(_inst);
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
			if (Format != TexFormat.Rgba32 && Format != TexFormat.Rgba32Linear)
			{ 
				Log.Err("Can't set a {0} format texture from Color32 data!", Format);
				return;
			}
			NativeAPI.tex_set_colors(_inst, width, height, data);
		}
		/// <summary>Set the texture's pixels using a color array! This 
		/// function sould only be called on textures with a format of Rgba128.
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
				Log.Err("Can't set a {0} format texture from Color data!", Format);
				return;
			}
			NativeAPI.tex_set_colors(_inst, width, height, data);
		}
		/// <summary>Set the texture's pixels using a scalar array! This 
		/// function sould only be called on textures with a format of R8.
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
			if (Format != TexFormat.R8)
			{
				Log.Err("Can't set a {0} format texture from byte data!", Format);
				return;
			}
			NativeAPI.tex_set_colors(_inst, width, height, data);
		}
		/// <summary>Set the texture's pixels using a scalar array! This 
		/// function sould only be called on textures with a format of R16.
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
			if (Format != TexFormat.R16)
			{
				Log.Err("Can't set a {0} format texture from ushort data!", Format);
				return;
			}
			NativeAPI.tex_set_colors(_inst, width, height, data);
		}
		/// <summary>Set the texture's pixels using a scalar array! This 
		/// function sould only be called on textures with a format of R32.
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
				Log.Err("Can't set a {0} format texture from Color data!", Format);
				return;
			}
			NativeAPI.tex_set_colors(_inst, width, height, data);
		}
		#endregion

		#region Static Methods

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
		/// the poles all streetched out. It uses some fancy shaders and
		/// texture blitting to create 6 faces from the equirectangular 
		/// image.</summary>
		/// <param name="equirectangularCubemap">Filename of the equirectangular
		/// image.</param>
		/// <param name="sRGBData">Is this image color data in sRGB format, 
		/// or is it normal/metal/rough/data that's not for direct display?
		/// sRGB colors get converted to linear color space on the graphics
		/// card, so getting this right can have a big impact on visuals.
		/// </param>
		/// <returns>A Cubemap texture asset!</returns>
		public static Tex FromCubemapEquirectangular(string equirectangularCubemap, bool sRGBData = true)
		{
			IntPtr tex = NativeAPI.tex_create_cubemap_file(equirectangularCubemap, sRGBData, IntPtr.Zero);
			return tex == IntPtr.Zero ? null : new Tex(tex);
		}

		/// <summary>Creates a cubemap texture from a single equirectangular 
		/// image! You know, the ones that look like an unwrapped globe with
		/// the poles all streetched out. It uses some fancy shaders and texture 
		/// blitting to create 6 faces from the equirectangular image.</summary>
		/// <param name="equirectangularCubemap">Filename of the equirectangular 
		/// image.</param>
		/// <param name="lightingInfo">An out value that represents the 
		/// lighting information scraped from the cubemap! This can then be
		/// passed to `Renderer.SkyLight`.</param>
		/// <param name="sRGBData">Is this image color data in sRGB format, 
		/// or is it normal/metal/rough/data that's not for direct display? 
		/// sRGB colors get converted to linear color space on the graphics
		/// card, so getting this right can have a big impact on visuals.
		/// </param>
		/// <returns>A Cubemap texture asset!</returns>
		public static Tex FromCubemapEquirectangular(string equirectangularCubemap, out SphericalHarmonics lightingInfo, bool sRGBData = true)
		{
			IntPtr tex = NativeAPI.tex_create_cubemap_file(equirectangularCubemap, sRGBData, out lightingInfo);
			return tex == IntPtr.Zero ? null : new Tex(tex);
		}

		/// <summary>Loads an image file directly into a texture! Supported
		/// formats are: jpg, png, tga, bmp, psd, gif, hdr, pic. Asset Id 
		/// will be the same as the filename.</summary>
		/// <param name="file">An absolute filename, or a filename relative 
		/// to the assets folder. Supports jpg, png, tga, bmp, psd, gif, hdr,
		/// pic</param>
		/// <param name="sRGBData">Is this image color data in sRGB format,
		/// or is it normal/metal/rough/data that's not for direct display?
		/// sRGB colors get converted to linear color space on the graphics
		/// card, so getting this right can have a big impact on visuals.</param>
		/// <returns>A Tex asset from the given file, or null if it failed to
		/// load.</returns>
		public static Tex FromFile(string file, bool sRGBData = true)
		{
			IntPtr inst = NativeAPI.tex_create_file(file, sRGBData);
			return inst == IntPtr.Zero ? null : new Tex(inst);
		}

		/// <summary>Loads an image file stored in memory directly into a 
		/// texture! Supported formats are: jpg, png, tga, bmp, psd, gif, 
		/// hdr, pic. Asset Id will be the same as the filename.</summary>
		/// <param name="data">The binary data of an image file, this is NOT 
		/// a raw RGB color array!</param>
		/// <param name="sRGBData">Is this image color data in sRGB format, 
		/// or is it normal/metal/rough/data that's not for direct display? 
		/// sRGB colors get converted to linear color space on the graphics
		/// card, so getting this right can have a big impact on visuals.</param>
		/// <returns>A Tex asset from the given file, or null if it failed to
		/// load.</returns>
		public static Tex FromMemory(in byte[] data, bool sRGBData = true)
		{
			IntPtr inst = NativeAPI.tex_create_mem(data, data.Length, sRGBData);
			return inst == IntPtr.Zero ? null : new Tex(inst);
		}

		/// <summary>Creates a cubemap texture from 6 different image files! 
		/// If you have a single equirectangular image, use Tex.FromEquirectangular 
		/// instead. Asset Id will be the first filename.</summary>
		/// <param name="cubeFaceFiles_xxyyzz">6 image filenames, in order of
		/// +X, -X, +Y, -Y, +Z, -Z.</param>
		/// <param name="sRGBData">Is this image color data in sRGB format, 
		/// or is it normal/metal/rough/data that's not for direct display? 
		/// sRGB colors get converted to linear color space on the graphics
		/// card, so getting this right can have a big impact on visuals.
		/// </param>
		/// <returns>A Tex asset from the given files, or null if any failed 
		/// to load.</returns>
		public static Tex FromCubemapFile(string[] cubeFaceFiles_xxyyzz, bool sRGBData = true)
		{
			if (cubeFaceFiles_xxyyzz.Length != 6)
				Log.Err("To create a cubemap, you must have exactly 6 images!");
			IntPtr inst = NativeAPI.tex_create_cubemap_files(cubeFaceFiles_xxyyzz, sRGBData, IntPtr.Zero);
			return inst == IntPtr.Zero ? null : new Tex(inst);
		}

		/// <summary>Creates a cubemap texture from 6 different image files! 
		/// If you have a single equirectangular image, use Tex.FromEquirectangular 
		/// instead. Asset Id will be the first filename.</summary>
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
		/// <returns>A Tex asset from the given files, or null if any failed 
		/// to load.</returns>
		public static Tex FromCubemapFile(string[] cubeFaceFiles_xxyyzz, out SphericalHarmonics lightingInfo, bool sRGBData = true)
		{
			if (cubeFaceFiles_xxyyzz.Length != 6)
				Log.Err("To create a cubemap, you must have exactly 6 images!");
			IntPtr inst = NativeAPI.tex_create_cubemap_files(cubeFaceFiles_xxyyzz, sRGBData, out lightingInfo);
			return inst == IntPtr.Zero ? null : new Tex(inst);
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
		/// face! This generally doesn't need to be large, as SphericalHarmonics 
		/// typically contain pretty low frequency information.</param>
		/// <returns>A procedurally generated cubemap texture!</returns>
		public static Tex GenCubemap(in SphericalHarmonics lighting, int resolution = 16)
		{
			IntPtr tex = NativeAPI.tex_gen_cubemap_sh(lighting, resolution);
			return tex == IntPtr.Zero ? null : new Tex(tex);
		}

		#endregion
	}
}
