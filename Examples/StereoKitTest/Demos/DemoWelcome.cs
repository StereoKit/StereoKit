using StereoKit;

class DemoWelcome : ITest
{
	Sprite logo;
	float  logoHalfWidth;
	string message = "Welcome fellow developer!\n\nThis is the StereoKit test app, a collection of demos and tests that cover StereoKit's major features. Use this panel to navigate around the app, and enjoy!\n\nCheck behind you for some debugging tools :)";

	public void Initialize()
	{
		logo = Sprite.FromFile("StereoKitWide.png", SpriteType.Single);
		logoHalfWidth = logo.Aspect * 0.5f * 0.25f;
	}

	public void Update()
	{
		Hierarchy.Push(Matrix.T(-0.3f, 0.1f, -0.7f));
		logo.Draw(Matrix.TRS(V.XYZ(logoHalfWidth, 0.3f, 0), Quat.LookDir(0, 0, 1), V.XYZ(logo.Aspect,1,1)*0.25f));
		Text.Add(message, Matrix.TRS(V.XYZ(-logoHalfWidth, 0, 0), Quat.LookDir(0, 0, 1), 1.25f), V.XY(logoHalfWidth/1.25f, 0), TextFit.Wrap, TextAlign.TopLeft, TextAlign.TopLeft);
		Hierarchy.Pop();
	}

	public void Shutdown()
	{
	}
}