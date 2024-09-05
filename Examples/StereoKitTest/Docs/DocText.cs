// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

using StereoKit;

class DocText : ITest
{
	TextStyle style = TextStyle.FromFont(Font.FromFile("aileron_font.ttf"), 0.2f, Color.White);

	void ShowTextStyleInfo()
	{
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

		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(size .XY0/-2 + V.XYZ(0, 0,    0.001f), size .XY0 + V.XYZ(0, 0, 0.001f)), colLayoutArea);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(sizeR.XY0/-2 + V.XYZ(0, yOff, 0.002f), sizeR.XY0 + V.XYZ(0, 0, 0.001f)), colRenderArea);

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
	}

	public void Step()
	{
		Hierarchy.Push(Matrix.TR(0, 0, -0.5f, Quat.LookDir(0, 0, 1)));

		ShowTextStyleInfo();
		Hierarchy.Pop();

		Vec2 s = Text.SizeLayout("lÔTy", style);
		Tests.Screenshot("TextStyleInfo.jpg", 400, 400, V.XYZ(s.x/2, s.y/-2, -0.18f), V.XYZ(s.x/2, s.y/-2, -0.5f));
	}

	public void Initialize() {}
	public void Shutdown  () {}
}