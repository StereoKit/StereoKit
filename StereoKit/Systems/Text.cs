using System.Runtime.InteropServices;

namespace StereoKit
{
	/// <summary>A text style is a font plus size/color/material parameters,
	/// and are used to keep text looking more consistent through the
	/// application by encouraging devs to re-use styles throughout the
	/// project. See Text.MakeStyle for making a TextStyle object.</summary>
	[StructLayout(LayoutKind.Sequential)]
	public struct TextStyle
	{
		internal uint _id;

		/// <summary>This provides a reference to the Material used by this
		/// style, so you can override certain features! Note that if you're
		/// creating TextStyles with manually provided Materials, this 
		/// Material may not be unique to this style.</summary>
		public Material Material => new Material(NativeAPI.text_style_get_material(this));

		/// <summary>Returns the maximum height of a text character using
		/// this style, in meters.</summary>
		public float CharHeight => NativeAPI.text_style_get_char_height(this);

		/// <summary>This is the default text style used by StereoKit.</summary>
		public static TextStyle Default { get => new TextStyle { _id = 0 }; }
	}

	/// <summary>A collection of functions for rendering and working with text.
	/// These are a lower level access to text rendering than the UI text
	/// functions, and are completely unaware of the UI code.</summary>
	public static class Text
	{
		/// <summary>Create a text style for use with other text functions! A
		/// text style is a font plus size/color/material parameters, and are
		/// used to keep text looking more consistent through the application
		/// by encouraging devs to re-use styles throughout the project.
		/// 
		/// This overload will create a unique Material for this style based
		/// on Default.ShaderFont.</summary>
		/// <param name="font">Font asset you want attached to this style.
		/// </param>
		/// <param name="characterHeightMeters">Height of a text glyph in
		/// meters. StereoKit currently bases this on the letter 'T'.</param>
		/// <param name="colorGamma">The gamma space color of the text
		/// style. This will be embedded in the vertex color of the text
		/// mesh.</param>
		/// <returns>A text style id for use with text rendering functions.
		/// </returns>
		public static TextStyle MakeStyle(Font font, float characterHeightMeters, Color colorGamma)
			=> NativeAPI.text_make_style(font._inst, characterHeightMeters, colorGamma);

		/// <summary>Create a text style for use with other text functions! A
		/// text style is a font plus size/color/material parameters, and are
		/// used to keep text looking more consistent through the application
		/// by encouraging devs to re-use styles throughout the project.
		/// 
		/// This overload will create a unique Material for this style based
		/// on the provided Shader.</summary>
		/// <param name="font">Font asset you want attached to this style.
		/// </param>
		/// <param name="characterHeightMeters">Height of a text glyph in
		/// meters. StereoKit currently bases this on the letter 'T'.</param>
		/// <param name="shader">This style will create and use a unique
		/// Material based on the Shader that you provide here.</param>
		/// <param name="colorGamma">The gamma space color of the text
		/// style. This will be embedded in the vertex color of the text
		/// mesh.</param>
		/// <returns>A text style id for use with text rendering functions.
		/// </returns>
		public static TextStyle MakeStyle(Font font, float characterHeightMeters, Shader shader, Color colorGamma)
			=> NativeAPI.text_make_style_shader(font._inst, characterHeightMeters, shader._inst, colorGamma);

		/// <summary>Create a text style for use with other text functions! A
		/// text style is a font plus size/color/material parameters, and are
		/// used to keep text looking more consistent through the application
		/// by encouraging devs to re-use styles throughout the project.
		/// 
		/// This overload allows you to set the specific Material that is 
		/// used. This can be helpful if you're keeping styles similar enough
		/// to re-use the material and save on draw calls. If you don't know
		/// what that means, then prefer using the overload that takes a 
		/// Shader, or takes neither a Shader nor a Material!</summary>
		/// <param name="font">Font asset you want attached to this style.
		/// </param>
		/// <param name="characterHeightMeters">Height of a text glyph in
		/// meters. StereoKit currently bases this on the letter 'T'.</param>
		/// <param name="material">Which material should be used to render
		/// the text with? Note that this does NOT duplicate the material, so
		/// some parameters of this Material instance will get overwritten, 
		/// like the texture used for the glyph atlas. You should either use
		/// a new Material, or a Material that was already used with this
		/// same font.</param>
		/// <param name="colorGamma">The gamma space color of the text
		/// style. This will be embedded in the vertex color of the text
		/// mesh.</param>
		/// <returns>A text style id for use with text rendering functions.
		/// </returns>
		public static TextStyle MakeStyle(Font font, float characterHeightMeters, Material material, Color colorGamma)
			=> NativeAPI.text_make_style_mat(font._inst, characterHeightMeters, material._inst, colorGamma);

		/// <summary>Renders text at the given location! Must be called every
		/// frame you want this text to be visible.</summary>
		/// <param name="text">What text should be drawn?</param>
		/// <param name="transform">A Matrix representing the transform of the
		/// text mesh! Try Matrix.TRS.</param>
		/// <param name="style">Style information for rendering, see
		/// Text.MakeStyle or the TextStyle object.</param>
		/// <param name="position">How should the text's bounding rectangle be
		/// positioned relative to the transform?</param>
		/// <param name="align">How should the text be aligned within the
		/// text's bounding rectangle?</param>
		/// <param name="offX">An additional offset on the X axis.</param>
		/// <param name="offY">An additional offset on the Y axis.</param>
		/// <param name="offZ">An additional offset on the Z axis.</param>
		public static void Add(string text, Matrix transform, TextStyle style, TextAlign position = TextAlign.Center, TextAlign align = TextAlign.Center, float offX=0, float offY=0, float offZ=0) 
			=> NativeAPI.text_add_at_16(text, transform, style._id, position, align, offX, offY, offZ, Color.White);

		/// <summary>Renders text at the given location! Must be called every
		/// frame you want this text to be visible.</summary>
		/// <param name="text">What text should be drawn?</param>
		/// <param name="transform">A Matrix representing the transform of the
		/// text mesh! Try Matrix.TRS.</param>
		/// <param name="position">How should the text's bounding rectangle be
		/// positioned relative to the transform?</param>
		/// <param name="align">How should the text be aligned within the
		/// text's bounding rectangle?</param>
		/// <param name="offX">An additional offset on the X axis.</param>
		/// <param name="offY">An additional offset on the Y axis.</param>
		/// <param name="offZ">An additional offset on the Z axis.</param>
		public static void Add(string text, Matrix transform, TextAlign position = TextAlign.Center, TextAlign align = TextAlign.Center, float offX = 0, float offY = 0, float offZ = 0)
			=> NativeAPI.text_add_at_16(text, transform, 0, position, align, offX, offY, offZ, Color.White);

		/// <inheritdoc cref="Add(string, Matrix, TextStyle, TextAlign, TextAlign, float, float, float)"/>
		/// <param name="vertexTintLinear">The vertex color of the text gets
		/// multiplied by this color. This is a linear color value, not a gamma
		/// corrected color value.</param>
		public static void Add(string text, Matrix transform, TextStyle style, Color vertexTintLinear, TextAlign position = TextAlign.Center, TextAlign align = TextAlign.Center, float offX = 0, float offY = 0, float offZ = 0)
			=> NativeAPI.text_add_at_16(text, transform, style._id, position, align, offX, offY, offZ, vertexTintLinear);

		/// <inheritdoc cref="Add(string, Matrix, TextAlign, TextAlign, float, float, float)"/>
		/// <param name="vertexTintLinear">The vertex color of the text gets
		/// multiplied by this color. This is a linear color value, not a gamma
		/// corrected color value.</param>
		public static void Add(string text, Matrix transform, Color vertexTintLinear, TextAlign position = TextAlign.Center, TextAlign align = TextAlign.Center, float offX = 0, float offY = 0, float offZ = 0)
			=> NativeAPI.text_add_at_16(text, transform, 0, position, align, offX, offY, offZ, vertexTintLinear);

		/// <summary>Renders text at the given location! Must be called every
		/// frame you want this text to be visible.</summary>
		/// <param name="text">What text should be drawn?</param>
		/// <param name="transform">A Matrix representing the transform of the
		/// text mesh! Try Matrix.TRS.</param>
		/// <param name="size">This is the Hierarchy space rectangle that the
		/// text should try to fit inside of. This allows for text wrapping or
		/// scaling based on the value provided to the 'fit' parameter.</param>
		/// <param name="fit">Describe how the text should behave when one of
		/// its size dimensions conflicts with the provided 'size' parameter.
		/// </param>
		/// <param name="style">Style information for rendering, see
		/// Text.MakeStyle or the TextStyle object.</param>
		/// <param name="position">How should the text's bounding rectangle be
		/// positioned relative to the transform?</param>
		/// <param name="align">How should the text be aligned within the
		/// text's bounding rectangle?</param>
		/// <param name="offX">An additional offset on the X axis.</param>
		/// <param name="offY">An additional offset on the Y axis.</param>
		/// <param name="offZ">An additional offset on the Z axis.</param>
		/// <returns>Returns the vertical space used by this text.</returns>
		public static float Add(string text, Matrix transform, Vec2 size, TextFit fit, TextStyle style, TextAlign position = TextAlign.Center, TextAlign align = TextAlign.Center, float offX = 0, float offY = 0, float offZ = 0)
			=> NativeAPI.text_add_in_16(text, transform, size, fit, style._id, position, align, offX, offY, offZ, Color.White);

		/// <summary>Renders text at the given location! Must be called every
		/// frame you want this text to be visible.</summary>
		/// <param name="text">What text should be drawn?</param>
		/// <param name="transform">A Matrix representing the transform of the
		/// text mesh! Try Matrix.TRS.</param>
		/// <param name="size">This is the Hierarchy space rectangle that the
		/// text should try to fit inside of. This allows for text wrapping or
		/// scaling based on the value provided to the 'fit' parameter.</param>
		/// <param name="fit">Describe how the text should behave when one of
		/// its size dimensions conflicts with the provided 'size' parameter.
		/// </param>
		/// <param name="position">How should the text's bounding rectangle be
		/// positioned relative to the transform?</param>
		/// <param name="align">How should the text be aligned within the
		/// text's bounding rectangle?</param>
		/// <param name="offX">An additional offset on the X axis.</param>
		/// <param name="offY">An additional offset on the Y axis.</param>
		/// <param name="offZ">An additional offset on the Z axis.</param>
		/// <returns>Returns the vertical space used by this text.</returns>
		public static float Add(string text, Matrix transform, Vec2 size, TextFit fit, TextAlign position = TextAlign.Center, TextAlign align = TextAlign.Center, float offX = 0, float offY = 0, float offZ = 0)
			=> NativeAPI.text_add_in_16(text, transform, size, fit, 0, position, align, offX, offY, offZ, Color.White);

		/// <inheritdoc cref="Add(string, Matrix, Vec2, TextFit, TextAlign, TextAlign, float, float, float)"/>
		/// <param name="vertexTintLinear">The vertex color of the text gets
		/// multiplied by this color. This is a linear color value, not a gamma
		/// corrected color value.</param>
		public static float Add(string text, Matrix transform, Vec2 size, TextFit fit, Color vertexTintLinear, TextAlign position = TextAlign.Center, TextAlign align = TextAlign.Center, float offX = 0, float offY = 0, float offZ = 0)
			=> NativeAPI.text_add_in_16(text, transform, size, fit, 0, position, align, offX, offY, offZ, vertexTintLinear);

		/// <inheritdoc cref="Add(string, Matrix, Vec2, TextFit, TextStyle, TextAlign, TextAlign, float, float, float)"/>
		/// <param name="vertexTintLinear">The vertex color of the text gets
		/// multiplied by this color. This is a linear color value, not a gamma
		/// corrected color value.</param>
		public static float Add(string text, Matrix transform, Vec2 size, TextFit fit, TextStyle style, Color vertexTintLinear, TextAlign position = TextAlign.Center, TextAlign align = TextAlign.Center, float offX = 0, float offY = 0, float offZ = 0)
			=> NativeAPI.text_add_in_16(text, transform, size, fit, style._id, position, align, offX, offY, offZ, vertexTintLinear);

		/// <summary>Sometimes you just need to know how much room some text
		/// takes up! This finds the size of the text in meters when using the
		/// indicated style!</summary>
		/// <param name="text">Text you want to find the size of.</param>
		/// <param name="style">The visual style of the text, see
		/// Text.MakeStyle or the TextStyle object for more details.</param>
		/// <returns>The width and height of the text in meters.</returns>
		public static Vec2 Size(string text, TextStyle style)
			=> NativeAPI.text_size_16(text, style._id);

		/// <summary>Sometimes you just need to know how much room some text
		/// takes up! This finds the size of the text in meters when using the
		/// default style!</summary>
		/// <param name="text">Text you want to find the size of.</param>
		/// <returns>The width and height of the text in meters.</returns>
		public static Vec2 Size(string text)
			=> NativeAPI.text_size_16(text, 0);
	}
}