using StereoKit;
using System;

class TestText : ITest
{
	TextStyle style;
	string    shortText = "Some shÖrt text, yes.";
	string    longText  = "Twas brillig, and the slithy toves Did gyre and gimble in the wabe: All mimsy were the borogoves, And the mome raths outgrabe.\nBeware the Jabberwock, my son! The jaws that bite, the claws that catch! Beware the Jubjub bird, and shun The frumious Bandersnatch!";
	string    scenders  = "lÔTy\nlÔTy\nlÔTy";

	public void Initialize() {
		//style = TextStyle.FromFont(Font.FromFile("aileron_font.ttf"), 0.02f, Color.White);
		style = TextStyle.FromFont(Font.Default, 0.02f, Color.White);
		style.Size = 0.02f * (0.02f / style.CapHeight);
	}
	public void Shutdown() { }
	public void Step()
	{
		Quat r  = Quat.FromAngles(0, 180, 0);
		Vec3 at = new Vec3(0,-0.1f,-0.5f);
		Text.Add(scenders, Matrix.TR(at, r), style, TextAlign.TopLeft, TextAlign.TopLeft);
		Vec2 s = Text.SizeLayout(scenders, style);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2, at.z - 0.001f, new Vec3(s.x, s.y, 0.001f)), Color.Black);
		s = Text.SizeRender(s, style, out float offset);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2 + offset, at.z - 0.002f, new Vec3(s.x, s.y, 0.001f)), new Color(0.25f,0.5f,0.25f));

		DrawTextLineGuides(at + V.XY0(0, 0 * -style.Size * style.LineHeight), s.x, style);
		DrawTextLineGuides(at + V.XY0(0, 1 * -style.Size * style.LineHeight), s.x, style);
		DrawTextLineGuides(at + V.XY0(0, 2 * -style.Size * style.LineHeight), s.x, style);

		at = new Vec3(0, 0.0f, -0.5f);
		Text.Add(shortText, Matrix.TR(at, r), style, TextAlign.TopLeft, TextAlign.TopLeft);
		s = Text.SizeLayout(shortText, style);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2, at.z - 0.001f, new Vec3(s.x, s.y, 0.001f)), Color.Black);
		s = Text.SizeRender(s, style, out offset);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2 + offset, at.z - 0.002f, new Vec3(s.x, s.y, 0.001f)), new Color(0.25f, 0.5f, 0.25f));

		at = new Vec3(0, 0.1f, -0.5f);
		Text.Add(longText, Matrix.TR(at, r), style, TextAlign.TopLeft, TextAlign.TopLeft);
		s = Text.SizeLayout(longText, style);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2, at.z - 0.001f, new Vec3(s.x, s.y, 0.001f)), Color.Black);
		s = Text.SizeRender(s, style, out offset);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2 + offset, at.z - 0.002f, new Vec3(s.x, s.y, 0.001f)), new Color(0.25f, 0.5f, 0.25f));


		at = new Vec3(0, 0.16f, -0.5f);
		s = Text.SizeLayout(shortText, style, 0.5f);
		Text.Add(shortText, Matrix.TR(at, r), new Vec2(0.5f, s.y), TextFit.Wrap, style, TextAlign.TopLeft, TextAlign.TopLeft);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2, at.z - 0.001f, new Vec3(s.x, s.y, 0.001f)), Color.Black);
		s = Text.SizeRender(s, style, out offset);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2 + offset, at.z - 0.002f, new Vec3(s.x, s.y, 0.001f)), new Color(0.25f, 0.5f, 0.25f));

		at = new Vec3(0, 0.5f, -0.5f);
		s = Text.SizeLayout(longText, style, 0.5f);
		Text.Add(longText, Matrix.TR(at, r), new Vec2(0.5f,s.y), TextFit.Wrap, style, TextAlign.TopLeft, TextAlign.TopLeft);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2, at.z - 0.001f, new Vec3(s.x, s.y, 0.001f)), Color.Black);
		s = Text.SizeRender(s, style, out offset);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2 + offset, at.z - 0.002f, new Vec3(s.x, s.y, 0.001f)), new Color(0.25f, 0.5f, 0.25f));

		Tests.Screenshot("Tests/TextSize.jpg", 1, 800, 300, 75, V.XYZ(0.85f, 0.3f, 0.0f), V.XYZ(0.85f, 0.3f, -0.5f));
	}

	static void DrawTextLineGuides(Vec3 at, float width, TextStyle style)
	{
		Color32 capHeight = new Color32(150, 255, 150, 255);
		Color32 baseline  = new Color32(255, 255, 255, 255);
		Color32 ascender  = new Color32(255, 100, 100, 255);
		Color32 descender = new Color32(100, 100, 255, 255);

		Lines.Add(at+V.XYZ(-0.01f, -style.LineHeight*style.Size, 0), at + V.XYZ(width+0.01f, -style.LineHeight*style.Size, 0), baseline, 0.0002f);
		Lines.Add(at+V.XY0(0, 0                               ), at + V.XY0(width, 0                               ), ascender,  0.0002f);
		Lines.Add(at+V.XY0(0, -style.Ascender                 ), at + V.XY0(width, -style.Ascender                 ), baseline,  0.0002f);
		Lines.Add(at+V.XY0(0, -style.Ascender+style.CapHeight ), at + V.XY0(width, -style.Ascender+style.CapHeight ), capHeight, 0.0002f);
		Lines.Add(at+V.XY0(0, -style.Size                     ), at + V.XY0(width, -style.Size                     ), descender, 0.0002f);
	}
}
