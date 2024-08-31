using StereoKit;

class DocText : ITest
{
	TextStyle style = TextStyle.FromFont(Font.FromFile("aileron_font.ttf"), 0.2f, Color.White);

	void ShowTextStyleInfo()
	{
		// Some representative characters:
		// l frequently goes above CapHeight all the way to the Ascender.
		// Ô will go past the Ascender outside the layout bounds.
		// T goes the whole way from Baseline to CapHeight.
		// y will go all the way down to the descender.
		string text = "lÔTy";

		// Draw the text
		Text.Add(text, Matrix.Identity, style, TextAlign.TopLeft, TextAlign.TopLeft);

		// Show the bounding regions for the size of the text
		Color colLayoutArea = Color.Black;
		Color colRenderArea = new Color(0.25f, 0.5f, 0.25f);

		Vec2 size  = Text.SizeLayout(text, style);
		Vec2 sizeR = Text.SizeRender(size, style, out float yOff);

		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(size .XY0/-2 + V.XYZ(0, 0,    0.001f), size .XY0 + V.XYZ(0, 0, 0.001f)), colLayoutArea);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(sizeR.XY0/-2 + V.XYZ(0, yOff, 0.002f), sizeR.XY0 + V.XYZ(0, 0, 0.001f)), colRenderArea);

		// Show lines representing the typography units for this style
		Color32 colCapHeight  = new Color32(150, 255, 150, 255);
		Color32 colBaseline   = new Color32(255, 255, 255, 255);
		Color32 colAscender   = new Color32(255, 100, 100, 255);
		Color32 colDescender  = new Color32(100, 100, 255, 255);
		Color32 colLineHeight = new Color32(255, 255, 255, 255);

		float ascenderAt   =  0;
		float baselineAt   = -style.Ascender;
		float capHeightAt  = -style.Ascender + style.CapHeight;
		float descenderAt  = -style.Size;
		float lineHeightAt = -style.LineHeight * style.Size;

		Lines.Add(V.XY0(0,     ascenderAt  ), V.XY0(-size.x,       ascenderAt  ), colAscender,   0.005f);
		Lines.Add(V.XY0(0,     baselineAt  ), V.XY0(-size.x,       baselineAt  ), colBaseline,   0.005f);
		Lines.Add(V.XY0(0,     capHeightAt ), V.XY0(-size.x,       capHeightAt ), colCapHeight,  0.005f);
		Lines.Add(V.XY0(0,     descenderAt ), V.XY0(-size.x,       descenderAt ), colDescender,  0.005f);
		Lines.Add(V.XY0(0.01f, lineHeightAt), V.XY0(-size.x-0.01f, lineHeightAt), colLineHeight, 0.005f);
	}

	public void Step()
	{
		Hierarchy.Push(Matrix.TR(0, 0, -0.5f, Quat.LookDir(0, 0, 1)));

		ShowTextStyleInfo();
		Hierarchy.Pop();

		Vec2 s = Text.SizeLayout("lÔTy", style);
		Tests.Screenshot("TextStyleInfo.jpg", 200, 200, V.XYZ(s.x/2, s.y/-2, -0.25f), V.XYZ(s.x/2, s.y/-2, -0.5f));
	}

	public void Initialize() {}
	public void Shutdown  () {}
}