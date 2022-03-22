using System;

namespace StereoKit {
	/// <summary>This class represents a text font asset! On the back-end, this asset
	/// is composed of a texture with font characters rendered to it, and a list of
	/// data about where, and how large those characters are on the texture.
	/// 
	/// This asset is used anywhere that text shows up, like in the UI or Text classes!
	/// </summary>
	public class Font
	{
		internal IntPtr _fontInst;

		private Font(IntPtr font)
		{
			_fontInst = font;
			if (_fontInst == IntPtr.Zero)
				Log.Err("Received an empty font!");
		}
		~Font()
		{
			if (_fontInst != IntPtr.Zero)
				NativeAPI.assets_releaseref_threadsafe(_fontInst);
		}

		/// <summary>Searches the asset list for a font with the given Id, returning null if
		/// none is found.</summary>
		/// <param name="fontId">Id of the font you're looking for.</param>
		/// <returns>An existing font asset, or null if none is found.</returns>
		public static Font Find(string fontId)
		{
			IntPtr fontInst = NativeAPI.font_find(fontId);
			if (fontInst == IntPtr.Zero)
			{
				Log.Write(LogLevel.Warning, "Couldn't find a font named {0}", fontId);
				return null;
			}
			return new Font(fontInst);
		}

		/// <summary>Loads a font and creates a font asset from it.</summary>
		/// <param name="fontFiles">A list of file addresses for the font! For
		/// example: 'C:/Windows/Fonts/segoeui.ttf'. If a glyph is not found,
		/// StereoKit will look in the next font file in the list.</param>
		/// <returns>A font from the given files, or null if all of the files
		/// failed to load properly! If any of the given files successfully 
		/// loads, then this font will be a valid asset.</returns>
		public static Font FromFile(params string[] fontFiles)
		{
			IntPtr inst = NativeAPI.font_create_files(fontFiles, fontFiles.Length);
			return inst == IntPtr.Zero ? null : new Font(inst);
		}

		/// <inheritdoc cref="StereoKit.Default.Font" />
		public static Font Default => StereoKit.Default.Font;
	}
}