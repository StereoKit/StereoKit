// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

using System;
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
		[Obsolete("Use TextStyle.LayoutHeight")]
		public float CharHeight
		{
			get => NativeAPI.text_style_get_layout_height(this);
			set => NativeAPI.text_style_set_layout_height(this, value);
		}

		/// <summary>(meters) Layout height is the height of the font's
		/// CapHeight, which is used for calculating the vertical height of the
		/// text when doing text layouts. This does _not_ include the height of
		/// the descender, nor does it represent the maximum possible height a
		/// glyph may extend upwards (use Text.SizeRender).</summary>
		public float LayoutHeight
		{
			get => NativeAPI.text_style_get_layout_height(this);
			set => NativeAPI.text_style_set_layout_height(this, value);
		}

		/// <summary>(meters) Height from the layout descender to the layout
		/// ascender. This is most equivalent to the 'font-size' in CSS or
		/// other text layout tools. Since ascender and descenders can vary a
		/// lot, using LayoutHeight in many cases can lead to more consistency
		/// in the long run.</summary>
		public float TotalHeight
		{
			get => NativeAPI.text_style_get_total_height(this);
			set => NativeAPI.text_style_set_total_height(this, value);
		}

		/// <summary>(meters) The height of a standard captial letter, such as
		/// 'H' or 'T'.</summary>
		public float CapHeight => NativeAPI.text_style_get_cap_height(this);

		/// <summary>(meters) The layout ascender of the font, this is the
		/// height of the "tallest" glyphs as far as layout is concerned.
		/// Characters such as 'l' typically rise above the CapHeight, and this
		/// value usually matches this height. Some glyphs such as those with
		/// hats or umlauts will almost always be taller than this height (see
		/// Text.SizeRender), but this is not used when laying out characters.
		/// </summary>
		public float Ascender => NativeAPI.text_style_get_ascender(this);
		/// <summary>(meters) The layout descender of the font, this is the
		/// positive height below the baseline</summary>
		public float Descender => NativeAPI.text_style_get_descender(this);

		/// <summary>This is the space a full line of text takes, from baseline
		/// to baseline, as a 0-1 percentage of the font's TotalHeight. This
		/// is similar to CSS line-height, a value of 1.0 means this line's
		/// descenders will squish up adjacent to the next line's tallest
		/// ascenders.</summary>
		public float LineHeightPct
		{
			get => NativeAPI.text_style_get_line_height_pct(this);
			set => NativeAPI.text_style_set_line_height_pct(this, value);
		}

		/// <summary>This is the default text style used by StereoKit.</summary>
		public static TextStyle Default { get => new TextStyle { _id = 0 }; }

		/// <summary>Create a text style for use with other text functions! A
		/// text style is a font plus size/color/material parameters, and are
		/// used to keep text looking more consistent through the application
		/// by encouraging devs to re-use styles throughout the project.
		/// 
		/// This overload will create a unique Material for this style based
		/// on Default.ShaderFont.</summary>
		/// <param name="font">Font asset you want attached to this style.
		/// </param>
		/// <param name="layoutHeightMeters">Height of a text glyph in
		/// meters. StereoKit currently bases this on CapHeight.
		/// </param>
		/// <param name="colorGamma">The gamma space color of the text
		/// style. This will be embedded in the vertex color of the text
		/// mesh.</param>
		/// <returns>A text style id for use with text rendering functions.
		/// </returns>
		public static TextStyle FromFont(Font font, float layoutHeightMeters, Color colorGamma)
			=> NativeAPI.text_make_style(font._inst, layoutHeightMeters, colorGamma);

		/// <summary>Create a text style for use with other text functions! A
		/// text style is a font plus size/color/material parameters, and are
		/// used to keep text looking more consistent through the application
		/// by encouraging devs to re-use styles throughout the project.
		/// 
		/// This overload will create a unique Material for this style based
		/// on the provided Shader.</summary>
		/// <param name="font">Font asset you want attached to this style.
		/// </param>
		/// <param name="layoutHeightMeters">Height of a text glyph in
		/// meters. StereoKit currently bases this on CapHeight.
		/// </param>
		/// <param name="shader">This style will create and use a unique
		/// Material based on the Shader that you provide here.</param>
		/// <param name="colorGamma">The gamma space color of the text
		/// style. This will be embedded in the vertex color of the text
		/// mesh.</param>
		/// <returns>A text style id for use with text rendering functions.
		/// </returns>
		public static TextStyle FromFont(Font font, float layoutHeightMeters, Shader shader, Color colorGamma)
			=> NativeAPI.text_make_style_shader(font._inst, layoutHeightMeters, shader._inst, colorGamma);

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
		/// <param name="layoutHeightMeters">Height of a text glyph in
		/// meters. StereoKit currently bases this on CapHeight.
		/// </param>
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
		public static TextStyle FromFont(Font font, float layoutHeightMeters, Material material, Color colorGamma)
			=> NativeAPI.text_make_style_mat(font._inst, layoutHeightMeters, material._inst, colorGamma);
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
		/// <param name="layoutHeightMeters">Height of a text glyph in
		/// meters. StereoKit currently bases this on CapHeight.
		/// </param>
		/// <param name="colorGamma">The gamma space color of the text
		/// style. This will be embedded in the vertex color of the text
		/// mesh.</param>
		/// <returns>A text style id for use with text rendering functions.
		/// </returns>
		public static TextStyle MakeStyle(Font font, float layoutHeightMeters, Color colorGamma)
			=> NativeAPI.text_make_style(font._inst, layoutHeightMeters, colorGamma);

		/// <summary>Create a text style for use with other text functions! A
		/// text style is a font plus size/color/material parameters, and are
		/// used to keep text looking more consistent through the application
		/// by encouraging devs to re-use styles throughout the project.
		/// 
		/// This overload will create a unique Material for this style based
		/// on the provided Shader.</summary>
		/// <param name="font">Font asset you want attached to this style.
		/// </param>
		/// <param name="layoutHeightMeters">Height of a text glyph in
		/// meters. StereoKit currently bases this on CapHeight.
		/// </param>
		/// <param name="shader">This style will create and use a unique
		/// Material based on the Shader that you provide here.</param>
		/// <param name="colorGamma">The gamma space color of the text
		/// style. This will be embedded in the vertex color of the text
		/// mesh.</param>
		/// <returns>A text style id for use with text rendering functions.
		/// </returns>
		public static TextStyle MakeStyle(Font font, float layoutHeightMeters, Shader shader, Color colorGamma)
			=> NativeAPI.text_make_style_shader(font._inst, layoutHeightMeters, shader._inst, colorGamma);

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
		/// <param name="layoutHeightMeters">Height of a text glyph in
		/// meters. StereoKit currently bases this on CapHeight.
		/// </param>
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
		public static TextStyle MakeStyle(Font font, float layoutHeightMeters, Material material, Color colorGamma)
			=> NativeAPI.text_make_style_mat(font._inst, layoutHeightMeters, material._inst, colorGamma);

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
			=> NativeAPI.text_add_at_16(text, transform, style, position, align, offX, offY, offZ, Color.White);

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
			=> NativeAPI.text_add_at_16(text, transform, TextStyle.Default, position, align, offX, offY, offZ, Color.White);

		/// <inheritdoc cref="Add(string, Matrix, TextStyle, TextAlign, TextAlign, float, float, float)"/>
		/// <param name="vertexTintLinear">The vertex color of the text gets
		/// multiplied by this color. This is a linear color value, not a gamma
		/// corrected color value.</param>
		public static void Add(string text, Matrix transform, TextStyle style, Color vertexTintLinear, TextAlign position = TextAlign.Center, TextAlign align = TextAlign.Center, float offX = 0, float offY = 0, float offZ = 0)
			=> NativeAPI.text_add_at_16(text, transform, style, position, align, offX, offY, offZ, vertexTintLinear);

		/// <inheritdoc cref="Add(string, Matrix, TextAlign, TextAlign, float, float, float)"/>
		/// <param name="vertexTintLinear">The vertex color of the text gets
		/// multiplied by this color. This is a linear color value, not a gamma
		/// corrected color value.</param>
		public static void Add(string text, Matrix transform, Color vertexTintLinear, TextAlign position = TextAlign.Center, TextAlign align = TextAlign.Center, float offX = 0, float offY = 0, float offZ = 0)
			=> NativeAPI.text_add_at_16(text, transform, TextStyle.Default, position, align, offX, offY, offZ, vertexTintLinear);

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
			=> NativeAPI.text_add_in_16(text, transform, size, fit, style, position, align, offX, offY, offZ, Color.White);

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
			=> NativeAPI.text_add_in_16(text, transform, size, fit, TextStyle.Default, position, align, offX, offY, offZ, Color.White);

		/// <inheritdoc cref="Add(string, Matrix, Vec2, TextFit, TextAlign, TextAlign, float, float, float)"/>
		/// <param name="vertexTintLinear">The vertex color of the text gets
		/// multiplied by this color. This is a linear color value, not a gamma
		/// corrected color value.</param>
		public static float Add(string text, Matrix transform, Vec2 size, TextFit fit, Color vertexTintLinear, TextAlign position = TextAlign.Center, TextAlign align = TextAlign.Center, float offX = 0, float offY = 0, float offZ = 0)
			=> NativeAPI.text_add_in_16(text, transform, size, fit, TextStyle.Default, position, align, offX, offY, offZ, vertexTintLinear);

		/// <inheritdoc cref="Add(string, Matrix, Vec2, TextFit, TextStyle, TextAlign, TextAlign, float, float, float)"/>
		/// <param name="vertexTintLinear">The vertex color of the text gets
		/// multiplied by this color. This is a linear color value, not a gamma
		/// corrected color value.</param>
		public static float Add(string text, Matrix transform, Vec2 size, TextFit fit, TextStyle style, Color vertexTintLinear, TextAlign position = TextAlign.Center, TextAlign align = TextAlign.Center, float offX = 0, float offY = 0, float offZ = 0)
			=> NativeAPI.text_add_in_16(text, transform, size, fit, style, position, align, offX, offY, offZ, vertexTintLinear);

		/// <summary>Sometimes you just need to know how much room some text
		/// takes up! This finds the size of the text in meters when using the
		/// indicated style!</summary>
		/// <param name="text">Text you want to find the size of.</param>
		/// <param name="style">The visual style of the text, see
		/// Text.MakeStyle or the TextStyle object for more details.</param>
		/// <returns>The width and height of the text in meters.</returns>
		[Obsolete("Use Text.SizeLayout")]
		public static Vec2 Size(string text, TextStyle style)
			=> NativeAPI.text_size_layout_16(text, style);

		/// <summary>Sometimes you just need to know how much room some text
		/// takes up! This finds the size of the text in meters when using the
		/// default style!</summary>
		/// <param name="text">Text you want to find the size of.</param>
		/// <returns>The width and height of the text in meters.</returns>
		[Obsolete("Use Text.SizeLayout")]
		public static Vec2 Size(string text)
			=> NativeAPI.text_size_layout_16(text, TextStyle.Default);

		/// <summary>Need to know how much space text will take when
		/// constrained to a certain width? This will find it using the default
		/// text style!</summary>
		/// <param name="text">Text to measure the size of.</param>
		/// <param name="maxWidth">Width of the available space in meters.</param>
		/// <returns>The size that this text will take up, in meters! Width
		/// will be the same as maxWidth as long as the text takes more than
		/// one line, and height will be the total height of the text.</returns>
		[Obsolete("Use Text.SizeLayout")]
		public static Vec2 Size(string text, float maxWidth)
			=> NativeAPI.text_size_layout_constrained_16(text, TextStyle.Default, maxWidth);

		/// <summary>Need to know how much space text will take when
		/// constrained to a certain width? This will find it using the
		/// indicated text style!</summary>
		/// <param name="text">Text to measure the size of.</param>
		/// <param name="maxWidth">Width of the available space in meters.</param>
		/// <returns>The size that this text will take up, in meters! Width
		/// will be the same as maxWidth as long as the text takes more than
		/// one line, and height will be the total height of the text.</returns>
		[Obsolete("Use Text.SizeLayout")]
		public static Vec2 Size(string text, TextStyle style, float maxWidth)
			=> SizeLayout(text, style, maxWidth);

		/// <summary>Sometimes you just need to know how much room some text
		/// takes up! This finds the layout size of the text in meters when
		/// using the indicated style!  This does not include ascender and
		/// descender size, so rendering using this as a clipping size will
		/// result in ascenders and descenders getting clipped.</summary>
		/// <param name="text">Text you want to find the size of.</param>
		/// <param name="style">The visual style of the text, see
		/// Text.MakeStyle or the TextStyle object for more details.</param>
		/// <returns>The width and height of the text in meters.</returns>
		public static Vec2 SizeLayout(string text, TextStyle style)
			=> NativeAPI.text_size_layout_16(text, style);

		/// <summary>Need to know how much layout space text will take when
		/// constrained to a certain width? This will find it using the
		/// indicated text style! This does not include ascender and descender
		/// size, so rendering using this as a clipping size will result in
		/// ascenders and descenders getting clipped.</summary>
		/// <param name="text">Text to measure the size of.</param>
		/// <param name="maxWidth">Width of the available space in meters.</param>
		/// <returns>The layoutsize that this text will take up, in meters!
		/// Width will be the same as maxWidth as long as the text takes more
		/// than one line, and height will be the total layout height of the
		/// text.</returns>
		public static Vec2 SizeLayout(string text, TextStyle style, float maxWidth)
			=> NativeAPI.text_size_layout_constrained_16(text, style, maxWidth);

		/// <summary>This modifies a text layout size with information related</summary>
		/// <param name="sizeLayout"></param>
		/// <param name="style"></param>
		/// <param name="yOffset"></param>
		/// <returns></returns>
		public static Vec2 SizeRender(Vec2 sizeLayout, TextStyle style, out float yOffset)
			=> NativeAPI.text_size_render(sizeLayout, style, out yOffset);
	}
}