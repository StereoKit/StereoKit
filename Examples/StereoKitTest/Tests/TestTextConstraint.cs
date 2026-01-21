using StereoKit;
using System;

class TestTextConstraint : ITest
{
	public void Step()
	{
		Show("one\ntwo\nthree\nfour", new Vec2(0.1f, 0), 0.006f);
		Show("one\n\ntwo\nthree",     new Vec2(0.2f, 0), 0.03f);
		Show("Vvardenfell",           new Vec2(0,    0), 0.01f);

		Tests.Screenshot("Tests/TextConstraint.jpg", 200, 400, 30, new Vec3(0.1f,-0.25f,-1.2f), new Vec3(0.1f,-0.25f,0));
	}

	static void Show(string text, Vec2 at, float width)
	{
		Vec2 size = Text.SizeLayout(text, TextStyle.Default, width);
		Text.Add(text, Matrix.T(at.x, at.y, 0), new Vec2(width, 0), TextFit.Wrap | TextFit.Clip, Pivot.TopLeft, Align.TopLeft);
		Lines.Add(new Vec3(at.XY0), new Vec3(at.x, at.y - size.y,0), Color.Black, 0.001f);
		Lines.Add(new Vec3(at.x-size.x, at.y, 0), new Vec3((at-size).XY0), Color.Black, 0.001f);
	}

	public void Initialize() { }
	public void Shutdown() { }
}