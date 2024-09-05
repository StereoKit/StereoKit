// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

using StereoKit;

class TestText : ITest
{
	TextStyle style;
	string    shortText = "Some shÖrt text, yes.";
	string    longText  = "Twas brillig, and the slithy toves Did gyre and gimble in the wabe:\nAll mimsy were the borogoves, And the mome raths outgrabe.\nBeware the Jabberwock, my son! The jaws that bite, the claws that catch!\nBeware the Jubjub bird, and shun The frumious Bandersnatch!";
	string    scenders  = "lÔTy\nlÔTy\nlÔTy";

	public void Initialize() {
		style = TextStyle.FromFont(Font.FromFile("aileron_font.ttf"), 0.02f, Color.White);
		style.LineHeightPct = 1.2f;
	}
	public void Shutdown() { }
	public void Step()
	{
		Color colLayout = new Color(0.1f,  0.1f, 0.1f);
		Color colRender = new Color(0.25f, 0.5f, 0.25f);

		Quat r  = Quat.FromAngles(0, 180, 0);
		Vec3 at = new Vec3(0.52f,0.35f,-0.5f);
		Text.Add(scenders, Matrix.TR(at, r), style, TextAlign.TopLeft, TextAlign.TopLeft);
		Vec2 s = Text.SizeLayout(scenders, style);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2, at.z - 0.001f, new Vec3(s.x, s.y, 0.001f)), colLayout);
		s = Text.SizeRender(s, style, out float offset);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2 + offset, at.z - 0.002f, new Vec3(s.x, s.y, 0.001f)), colRender);

		DrawTextLineGuides(at + V.XY0(0, 0 * -style.TotalHeight * style.LineHeightPct), s.x, style);
		DrawTextLineGuides(at + V.XY0(0, 1 * -style.TotalHeight * style.LineHeightPct), s.x, style);
		DrawTextLineGuides(at + V.XY0(0, 2 * -style.TotalHeight * style.LineHeightPct), s.x, style);

		at = new Vec3(0.52f, 0.15f, -0.5f);
		s = Text.SizeLayout(shortText, style, 0.5f);
		Text.Add(shortText, Matrix.TR(at, r), new Vec2(0.5f, s.y), TextFit.Wrap, style, TextAlign.TopLeft, TextAlign.TopLeft);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2, at.z - 0.001f, new Vec3(s.x, s.y, 0.001f)), colLayout);
		s = Text.SizeRender(s, style, out offset);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2 + offset, at.z - 0.002f, new Vec3(s.x, s.y, 0.001f)), colRender);

		at = new Vec3(0, 0.35f, -0.5f);
		s = Text.SizeLayout(longText, style, 0.5f);
		Text.Add(longText, Matrix.TR(at, r), new Vec2(0.5f,s.y), TextFit.Wrap, style, TextAlign.TopLeft, TextAlign.TopLeft);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2, at.z - 0.001f, new Vec3(s.x, s.y, 0.001f)), colLayout);
		s = Text.SizeRender(s, style, out offset);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2 + offset, at.z - 0.002f, new Vec3(s.x, s.y, 0.001f)), colRender);

		at = new Vec3(0.52f, 0.2f, -0.5f);
		Text.Add(shortText, Matrix.TR(at, r), style, TextAlign.TopLeft, TextAlign.TopLeft);
		s = Text.SizeLayout(shortText, style);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2, at.z - 0.001f, new Vec3(s.x, s.y, 0.001f)), colLayout);
		s = Text.SizeRender(s, style, out offset);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2 + offset, at.z - 0.002f, new Vec3(s.x, s.y, 0.001f)), colRender);

		at = new Vec3(0, 0.05f, -0.5f);
		Text.Add(longText, Matrix.TR(at, r), style, TextAlign.TopLeft, TextAlign.TopLeft);
		s = Text.SizeLayout(longText, style);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2, at.z - 0.001f, new Vec3(s.x, s.y, 0.001f)), colLayout);
		s = Text.SizeRender(s, style, out offset);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2 + offset, at.z - 0.002f, new Vec3(s.x, s.y, 0.001f)), colRender);

		Tests.Screenshot("Tests/TextSize.jpg", 1, 800, 500, 75, V.XYZ(s.x/2, 0.14f, -0.13f), V.XYZ(s.x/2, 0.14f, -0.5f));
	}

	static void DrawTextLineGuides(Vec3 at, float width, TextStyle style)
	{
		Color32 colCapHeight = new Color32( 50, 255,  50, 255);
		Color32 colBaseline  = new Color32(255, 255, 255, 255);
		Color32 colAscender  = new Color32(255,  50,  50, 255);
		Color32 colDescender = new Color32( 50,  50, 255, 255);

		float baseline = -style.CapHeight;
		float top      = baseline+style.Ascender;
		Lines.Add(at+V.XYZ(-0.01f, top-style.LineHeightPct*style.TotalHeight, 0), at + V.XYZ(width+0.01f, top-style.LineHeightPct*style.TotalHeight, 0), colBaseline, 0.002f);
		Lines.Add(at+V.XY0(0, baseline                ), at + V.XY0(width, baseline                ), colBaseline,  0.0005f);
		Lines.Add(at+V.XY0(0, baseline+style.CapHeight), at + V.XY0(width, baseline+style.CapHeight), colCapHeight, 0.0005f);
		Lines.Add(at+V.XY0(0, baseline+style.Ascender ), at + V.XY0(width, baseline+style.Ascender ), colAscender,  0.0005f);
		Lines.Add(at+V.XY0(0, baseline-style.Descender), at + V.XY0(width, baseline-style.Descender), colDescender, 0.0005f);
	}
}
