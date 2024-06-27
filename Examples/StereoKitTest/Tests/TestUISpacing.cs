using StereoKit;

class TestUISpacing : ITest
{
	Material layoutBox;

	public void Initialize()
	{
		layoutBox = Material.Unlit.Copy();
		layoutBox.Transparency = Transparency.Add;
		layoutBox.DepthTest    = DepthTest.Always;
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
		Vec2 labelSize = new Vec2(0.04f, UI.LineHeight);

		Pose pose = new Pose(-0.25f,0,-0.5f, Quat.LookDir(0, 0, 1));
		UI.WindowBegin("Horizontal Spacing", ref pose, new Vec2(0.2f, 0.3f));
		ShowCmGrid();

		UI.Label($"{UI.LayoutRemaining.x:0.00}", labelSize); ShowLayout();
		UI.HSpace(0.02f); UI.Label($"{UI.LayoutRemaining.x:0.00}", labelSize); ShowLayout();
		UI.HSpace(0.04f); UI.Label($"{UI.LayoutRemaining.x:0.00}", labelSize); ShowLayout();

		UI.Label($"{UI.LayoutRemaining.x:0.00}", labelSize); ShowLayout();                   UI.SameLine(); UI.HSpace(0.02f); UI.Label($"{UI.LayoutRemaining.x:0.00}", labelSize); ShowLayout();
		UI.HSpace(0.02f); UI.Label($"{UI.LayoutRemaining.x:0.00}", labelSize); ShowLayout(); UI.SameLine(); UI.HSpace(0.02f); UI.Label($"{UI.LayoutRemaining.x:0.00}", labelSize); ShowLayout();
		UI.HSpace(0.04f); UI.Label($"{UI.LayoutRemaining.x:0.00}", labelSize); ShowLayout(); UI.SameLine(); UI.HSpace(0.02f); UI.Label($"{UI.LayoutRemaining.x:0.00}", labelSize); ShowLayout();

		UI.WindowEnd();


		pose = new Pose(0.0f,0,-0.5f, Quat.LookDir(0, 0, 1));
		UI.WindowBegin("Vertical Spacing", ref pose, new Vec2(0.2f, 0.35f));
		ShowCmGrid();

		UI.Label($"{UI.LayoutRemaining.y:0.00}", labelSize); ShowLayout();
		UI.VSpace(0.02f); UI.Label($"{UI.LayoutRemaining.y:0.00}", labelSize); ShowLayout();
		UI.VSpace(0.04f); UI.Label($"{UI.LayoutRemaining.y:0.00}", labelSize); ShowLayout(); UI.SameLine(); UI.Label($"{UI.LayoutRemaining.y:0.00}", labelSize); ShowLayout();

		UI.Label($"{UI.LayoutRemaining.y:0.00}", labelSize); ShowLayout(); UI.SameLine(); UI.VSpace(0.02f); UI.Label($"{UI.LayoutRemaining.y:0.00}", labelSize); ShowLayout();
		UI.VSpace(0.02f); UI.Label($"{UI.LayoutRemaining.y:0.00}", labelSize); ShowLayout(); UI.SameLine(); UI.VSpace(0.02f); UI.Label($"{UI.LayoutRemaining.y:0.00}", labelSize); ShowLayout();

		UI.WindowEnd();


		pose = new Pose(0.25f, 0, -0.5f, Quat.LookDir(0, 0, 1));
		UI.WindowBegin("Both Spacing", ref pose, new Vec2(0.2f, 0.2f));
		ShowCmGrid();

		UI.Label($"{UI.LayoutRemaining.y:0.00}x{UI.LayoutRemaining.y:0.00}", labelSize); ShowLayout();
		UI.VSpace(0.02f); UI.HSpace(0.04f); UI.Label($"{UI.LayoutRemaining.y:0.00}x{UI.LayoutRemaining.y:0.00}", labelSize); ShowLayout();
		UI.HSpace(0.02f); UI.VSpace(0.04f); UI.Label($"{UI.LayoutRemaining.y:0.00}x{UI.LayoutRemaining.y:0.00}", labelSize); ShowLayout();

		UI.WindowEnd();


		Tests.Screenshot("Tests/Spacing.jpg", 1, 600, 400, 35, V.XYZ(0, -0.15f, 0.25f), V.XYZ(0, -0.15f, 0));
	}

	void ShowLayout()
	{
		Mesh.Cube.Draw(layoutBox, Matrix.TS(UI.LayoutLast.center, UI.LayoutLast.dimensions), new Color(0.1f,0.1f,0.1f,0));
	}

	void ShowCmGrid()
	{
		int height = (int)(UI.LayoutRemaining.y / 0.01f);
		int width  = (int)(UI.LayoutRemaining.x / 0.01f);
		float d = -0.001f;

		float xMin = UI.LayoutAt.x;
		float xMax = UI.LayoutAt.x - UI.LayoutRemaining.x;
		float yStart = UI.LayoutAt.y;
		for (int y = 0; y <= height; y++)
			Lines.Add(new Vec3(xMin, yStart - y*0.01f, d), new Vec3(xMax, yStart - y * 0.01f, d), new Color32(255,255,255,40), 0.0005f);


		float yMin = UI.LayoutAt.y;
		float yMax = UI.LayoutAt.y - UI.LayoutRemaining.y;
		float xStart = UI.LayoutAt.x;
		for (int x = 0; x <= width; x++)
			Lines.Add(new Vec3(xStart - x*0.01f, yMin, d), new Vec3(xStart - x * 0.01f, yMax, d), new Color32(255,255,255,40), 0.0005f);
	}
}