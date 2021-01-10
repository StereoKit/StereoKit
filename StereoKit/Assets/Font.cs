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
				SK.ExecuteOnMain(()=>NativeAPI.font_release(_fontInst));
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
		/// <param name="fontFile">A file address for the font! For example: 'C:/Windows/Fonts/segoeui.ttf'</param>
		/// <returns>A font from the given file, or null if the file failed to load properly!</returns>
		public static Font FromFile(string fontFile)
		{
			IntPtr inst = NativeAPI.font_create(fontFile);
			return inst == IntPtr.Zero ? null : new Font(inst);
		}
	}
}