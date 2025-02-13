using StereoKit;

class TestText : ITest
{
	TextStyle style;
	string    shortText = "Some short text.";
	string    longText  = "Twas brillig, and the slithy toves Did gyre and gimble in the wabe: All mimsy were the borogoves, And the mome raths outgrabe.\nBeware the Jabberwock, my son! The jaws that bite, the claws that catch! Beware the Jubjub bird, and shun The frumious Bandersnatch!";

	public void Initialize() {
		style = TextStyle.FromFont(Font.FromFile("aileron_font.ttf"), 0.02f, Color.White);
	}
	public void Shutdown() { }
	public void Step()
	{
		Quat r = Quat.FromAngles(0, 180, 0);
		Vec3 at = new Vec3(0,0,-0.5f);
		Text.Add(shortText, Matrix.TR(at, r), style, TextAlign.TopLeft, TextAlign.TopLeft);
		Vec2 s = Text.Size(shortText);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2, at.z - 0.001f, new Vec3(s.x, s.y, 0.001f)), Color.Black);

		at = new Vec3(0, 0.1f, -0.5f);
		Text.Add(longText, Matrix.TR(at, r), style, TextAlign.TopLeft, TextAlign.TopLeft);
		s = Text.Size(longText);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2, at.z - 0.001f, new Vec3(s.x, s.y, 0.001f)), Color.Black) ;


		at = new Vec3(0, 0.2f, -0.5f);
		s = Text.Size(shortText, 0.5f);
		Text.Add(shortText, Matrix.TR(at, r), new Vec2(0.5f, s.y), TextFit.Wrap, style, TextAlign.TopLeft, TextAlign.TopLeft);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2, at.z - 0.001f, new Vec3(s.x, s.y, 0.001f)), Color.Black);

		at = new Vec3(0, 0.5f, -0.5f);
		s = Text.Size(longText, 0.5f);
		Text.Add(longText, Matrix.TR(at, r), new Vec2(0.5f,s.y), TextFit.Wrap, style, TextAlign.TopLeft, TextAlign.TopLeft);
		Mesh.Cube.Draw(Material.Unlit, Matrix.TS(at.x + s.x / 2, at.y - s.y / 2, at.z - 0.001f, new Vec3(s.x, s.y, 0.001f)), Color.Black);

		Tests.Screenshot("Tests/TextSize.jpg", 1, 800, 500, 75, V.XYZ(s.x/2, 0.14f, -0.1f), V.XYZ(s.x/2, 0.14f, -0.5f));
	}
}
