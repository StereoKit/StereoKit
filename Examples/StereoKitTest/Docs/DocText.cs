// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2024-2025 Nick Klingensmith
// Copyright (c) 2024-2025 Qualcomm Technologies, Inc.

using StereoKit;

class DocText : ITest
{
		public static void TextSizes()
	{
		/// :CodeSample: Text.SizeLayout Text.SizeRender
		/// ### Text Sizes
		/// 
		/// When you need to work with placing text, `Text.SizeLayout` and
		/// `Text.SizeRender` are the keys to the kingdom! `SizeLayout` will
		/// give you the size of your text as far as layout is generally
		/// concerned, while `SizeRender` will take your layout size and
		/// provide the total bounds that you need to watch out for! Some fonts
		/// can have absolutely _unreasonable_ ascenders and descenders for
		/// some of  their glyphs. Extreme cases can be a bit rare, so in
		/// general you'll only need to work with the layout size. Just watch
		/// out when you need to clip your text!
		/// 
		/// ![Text sizes]({{site.screen_url}}/Docs/Text_Sizes.jpg)
		/// _You can see here with Segoe UI, the ascender area for rendering looks ridiculous._
		/// 
		/// In this screenshot, the black area represents the layout size,
		/// while the gray area represents the render size. "lÔTy" is a decent
		/// set of characters to illustrate a pretty normal range of height
		/// variation.
		string text  = "lÔTy";
		TextStyle style = TextStyle.Default;

		Text.Add(text, Matrix.Identity, style, TextAlign.Center);

		// Calculate the text sizes! Layout size is used for placing text, but
		// render size indicates the total area where text could end up,
		// accounting for _extreme_ ascenders and descenders.
		Vec2 layoutSz = Text.SizeLayout(text,     style);
		Vec2 renderSz = Text.SizeRender(layoutSz, style, out float renderYOff);

		// Draw the layout size behind the text in black
		Mesh.Quad.Draw(Material.Unlit,
			Matrix.TS(V.XYZ(0, 0, 0.0001f), (-layoutSz).XY1),
			Color.Black);

		// Draw the render size behind the text in gray, note that we're
		// dividing the y offset by 2 because we're drawing from the _center_
		// of a quad rather than something like the top left.
		Mesh.Quad.Draw(Material.Unlit,
			Matrix.TS(V.XYZ(0, renderYOff/2.0f, 0.0002f), (-renderSz).XY1),
			new Color(0.2f,0.2f,0.2f));
		/// :End:
	}

	TextStyle style = TextStyle.FromFont(Font.FromFile("aileron_font.ttf"), 0.2f, Color.White);
	void ShowTextStyleInfo()
	{
		/// :CodeSample: TextStyle TextStyle.Ascender TextStyle.CapHeight TextStyle.Descender TextStyle.LineHeightPct TextStyle.TotalHeight
		/// ### TextStyle Info
		/// If you're doing advanced text layout, StereoKit does provide some
		/// information about the font underlying the TextStyle! Here's a
		/// quick sketch that shows what all that info represents:
		/// 
		/// ![Where style info lands on text]({{site.screen_url}}/Docs/Text_StyleInfo.jpg)
		/// 
		// Some representative characters:
		// l frequently goes above CapHeight all the way to the Ascender.
		// Ô will go past the Ascender outside the layout bounds, and slightly below the baseline.
		// T goes the whole way from Baseline to CapHeight.
		// y will go all the way down to the descender.
		string text = "lÔTy";

		// Draw the text
		Text.Add(text, Matrix.Identity, style, TextAlign.TopLeft, TextAlign.TopLeft);

		// Show the bounding regions for the size of the text
		Color colLayoutArea = new Color(0.1f,  0.1f, 0.1f);
		Color colRenderArea = new Color(0.25f, 0.5f, 0.25f);

		Vec2 size  = Text.SizeLayout(text, style);
		Vec2 sizeR = Text.SizeRender(size, style, out float yOff);

		Mesh.Quad.Draw(Material.Unlit, Matrix.TS(size .XY0/-2 + V.XYZ(0, 0,    0.0001f), size .XY1), colLayoutArea);
		Mesh.Quad.Draw(Material.Unlit, Matrix.TS(sizeR.XY0/-2 + V.XYZ(0, yOff, 0.0002f), sizeR.XY1), colRenderArea);

		// Show lines representing the typography units for this style
		Color32 colCapHeight  = new Color32( 50, 255,  50, 255);
		Color32 colBaseline   = new Color32(255, 255, 255, 255);
		Color32 colAscender   = new Color32(255,  50,  50, 255);
		Color32 colDescender  = new Color32( 50,  50, 255, 255);
		Color32 colLineHeight = new Color32(255, 255, 255, 255);

		float baselineAt   = -style.CapHeight;
		float ascenderAt   = baselineAt + style.Ascender;
		float capHeightAt  = baselineAt + style.CapHeight;
		float descenderAt  = baselineAt - style.Descender;
		float lineHeightAt = ascenderAt - style.LineHeightPct * style.TotalHeight;

		Lines.Add(V.XY0(0, ascenderAt  ), V.XY0(-size.x, ascenderAt  ), colAscender,   0.003f);
		Lines.Add(V.XY0(0, baselineAt  ), V.XY0(-size.x, baselineAt  ), colBaseline,   0.003f);
		Lines.Add(V.XY0(0, capHeightAt ), V.XY0(-size.x, capHeightAt ), colCapHeight,  0.003f);
		Lines.Add(V.XY0(0, descenderAt ), V.XY0(-size.x, descenderAt ), colDescender,  0.003f);
		Lines.Add(V.XY0(0, lineHeightAt), V.XY0(-size.x, lineHeightAt), colLineHeight, 0.003f);
		/// :End:
	}

	public void Step()
	{
		Hierarchy.Push(Matrix.TR(0, 0, -0.5f, Quat.LookDir(0, 0, 1)));
		ShowTextStyleInfo();
		Hierarchy.Pop();

		Vec2 s = Text.SizeLayout("lÔTy", style);
		Tests.Screenshot("Docs/Text_StyleInfo.jpg", 400, 400, 70, V.XY0(s.x / 2, s.y / -2), V.XYZ(s.x / 2, s.y / -2, -0.5f));

		Hierarchy.Push(Matrix.TR(0, 1, -0.5f, Quat.LookDir(0, 0, 1)));
		TextSizes();
		Hierarchy.Pop();

		s = Text.SizeLayout("lÔTy", TextStyle.Default);
		s = Text.SizeRender(s, TextStyle.Default, out float yOff);
		Tests.Screenshot("Docs/Text_Size.jpg", 400, 400, 15, V.XYZ(0, 1+yOff/2.0f, -0.18f), V.XYZ(0, 1+yOff/2.0f, -0.5f));
	}

	public void Initialize() {}
	public void Shutdown  () {}
}