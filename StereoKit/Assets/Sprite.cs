using System;
using System.Text;

namespace StereoKit
{
	/// <summary>A Sprite is an image that's set up for direct 2D rendering,
	/// without using a mesh or model! This is technically a wrapper over a
	/// texture, but it also includes atlasing functionality, which can be
	/// pretty important to performance! This is used a lot in UI, for image
	/// rendering.
	/// 
	/// Atlasing is not currently implemented, it'll swap to Single for now.
	/// But here's how it works!
	/// 
	/// StereoKit will batch your sprites into an atlas if you ask it to!
	/// This puts all the images on a single texture to significantly reduce
	/// draw calls when many images are present. Any time you add a sprite to
	/// an atlas, it'll be marked as dirty and rebuilt at the end of the
	/// frame. So it can be a good idea to add all your images to the atlas
	/// on initialize rather than during execution!
	/// 
	/// Since rendering is atlas based, you also have only one material per
	/// atlas. So this is why you might wish to put a sprite in one atlas or
	/// another, so you can apply different</summary>
	public class Sprite
	{
		internal IntPtr _inst;

		/// <summary>The aspect ratio of the sprite! This is width/height. 
		/// You may also be interested in the NormalizedDimensions property, 
		/// which are normalized to the 0-1 range.</summary>
		public float Aspect => NativeAPI.sprite_get_aspect(_inst);
		/// <summary>Width of the sprite, in pixels.</summary>
		public int   Width  => NativeAPI.sprite_get_width(_inst);
		/// <summary>Height of the sprite, in pixels.</summary>
		public int   Height => NativeAPI.sprite_get_height(_inst);
		/// <summary>Width and height of the sprite, normalized so the
		/// maximum value is 1.</summary>
		public Vec2 NormalizedDimensions 
			=> NativeAPI.sprite_get_dimensions_normalized(_inst);

		internal Sprite(IntPtr inst)
		{
			_inst = inst;
			if (_inst == IntPtr.Zero)
				Log.Err("Received an empty sprite!");
		}
		~Sprite()
		{
			if (_inst != IntPtr.Zero)
				NativeAPI.assets_releaseref_threadsafe(_inst);
		}

		/// <summary>Draw the sprite on a quad with the provided transform!
		/// </summary>
		/// <param name="transform">A Matrix describing a transform from 
		/// model space to world space.</param>
		/// <param name="color">Per-instance color data for this render item.
		/// </param>
		[Obsolete("This function does not correctly account for aspect ratio. Use a Draw method overload that takes TextAlign as a parameter.")]
		public void Draw(in Matrix transform, Color32 color) 
			=> NativeAPI.sprite_draw(_inst, transform, color);

		/// <summary>Draw the sprite on a quad with the provided transform!
		/// </summary>
		/// <param name="transform">A Matrix describing a transform from 
		/// model space to world space.</param>
		[Obsolete("This function does not correctly account for aspect ratio. Use a Draw method overload that takes TextAlign as a parameter.")]
		public void Draw(in Matrix transform)
			=> NativeAPI.sprite_draw(_inst, transform, Color32.White);

		/// <summary>Draws the sprite at the location specified by the
		/// transform matrix. A sprite is always sized in model space as 1 x
		/// Aspect meters on the x and y axes respectively, so scale
		/// appropriately. The 'position' attribute describes what corner of
		/// the sprite you're specifying the transform of.</summary>
		/// <param name="transform">A Matrix describing a transform from 
		/// model space to world space. A sprite is always sized in model
		/// space as 1 x Aspect meters on the x and y axes respectively, so
		/// scale appropriately and remember that your anchor position may
		/// affect the transform as well.</param>
		/// <param name="anchorPosition">Describes what corner of the sprite
		/// you're specifying the transform of. The 'Anchor' point or
		/// 'Origin' of the Sprite.</param>
		public void Draw(in Matrix transform, TextAlign anchorPosition)
			=> NativeAPI.sprite_draw_at(_inst, transform, anchorPosition, Color32.White);


		/// <inheritdoc cref="Draw(in Matrix, TextAlign)"/>
		/// <param name="linearColor">Per-instance color data for this render
		/// item. It is unmodified by StereoKit, and is generally interpreted
		/// as linear.</param>
		public void Draw(in Matrix transform, TextAlign anchorPosition, Color32 linearColor)
			=> NativeAPI.sprite_draw_at(_inst, transform, anchorPosition, linearColor);

		/// <summary>Create a sprite from an image file! This loads a Texture
		/// from file, and then uses that Texture as the source for the 
		/// Sprite.</summary>
		/// <param name="file">The filename of the image, an absolute 
		/// filename, or a filename relative to the assets folder. Supports 
		/// jpg, png, tga, bmp, psd, gif, hdr, pic.</param>
		/// <param name="type">Should this sprite be atlased, or an
		/// individual image? Adding this as an atlased image is better for
		/// performance, but will cause the atlas to be rebuilt! Images that
		/// take up too much space on the atlas, or might be loaded or 
		/// unloaded during runtime may be better as Single rather than
		/// Atlased!</param>
		/// <param name="atlasId">The name of which atlas the sprite should
		/// belong to, this is only relevant if the SpriteType is Atlased.
		/// </param>
		/// <returns>A Sprite asset, or null if the image failed to load!
		/// </returns>
		public static Sprite FromFile(string file, SpriteType type = SpriteType.Atlased, string atlasId = "default")
		{
			IntPtr inst = NativeAPI.sprite_create_file(NativeHelper.ToUtf8(file), type, atlasId);
			return inst == IntPtr.Zero ? null : new Sprite(inst);
		}

		/// <summary>Create a sprite from a Texture object!</summary>
		/// <param name="image">The texture to build a sprite from. Must be a
		/// valid, 2D image!</param>
		/// <param name="type">Should this sprite be atlased, or an
		/// individual image? Adding this as an atlased image is better for
		/// performance, but will cause the atlas to be rebuilt! Images that
		/// take up too much space on the atlas, or might be loaded or
		/// unloaded during runtime may be better as Single rather than
		/// Atlased!</param>
		/// <param name="atlasId">The name of which atlas the sprite should 
		/// belong to, this is only relevant if the SpriteType is Atlased.
		/// </param>
		/// <returns>A Sprite asset, or null if the image failed when adding
		/// to the sprite system!</returns>
		public static Sprite FromTex(Tex image, SpriteType type = SpriteType.Atlased, string atlasId = "default")
		{
			IntPtr inst = NativeAPI.sprite_create(image._inst, type, atlasId);
			return inst == IntPtr.Zero ? null : new Sprite(inst);
		}
	}
}
