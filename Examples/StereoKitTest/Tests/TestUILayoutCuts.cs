using StereoKit;

class TestUILayoutCuts : ITest
{
	public void Initialize() { Tests.RunForFrames(2); }
	public void Shutdown() { }

	public void Step()
	{
		EmptyParentTest();
		BasicTests();
		PracticalTest();
	}

	static void BasicTests()
	{
		Tests.Screenshot("Tests/LayoutCuts1.jpg", 1, 600, 400, 90, V.XYZ(0, -0.11f, 0.18f), V.XYZ(0, -0.11f, 0));

		Pose windowPose = new Pose(-0.1f,0,0, Quat.LookDir(-Vec3.Forward));
		UI.WindowBegin("Left layout cut", ref windowPose);

		UI.LayoutPushCut(UICut.Left, 0.1f);
		UI.Button("Left");
		UI.LayoutPop();
		UI.Button("Right");

		UI.WindowEnd();

		windowPose = new Pose(-0.1f,-0.1f,0, Quat.LookDir(-Vec3.Forward));
		UI.WindowBegin("Right layout cut", ref windowPose, V.XY(0.2f, 0.0f));

		UI.LayoutPushCut(UICut.Right, 0.1f);
		UI.Button("Right");
		UI.LayoutPop();
		UI.Button("Left");

		UI.WindowEnd();

		windowPose = new Pose(0.1f, 0, 0, Quat.LookDir(-Vec3.Forward));
		UI.WindowBegin("Top layout cut", ref windowPose);

		UI.LayoutPushCut(UICut.Top, 0.04f);
		UI.Button("Top");
		UI.LayoutPop();
		UI.Button("Bottom");

		UI.WindowEnd();

		windowPose = new Pose(0.1f, -0.16f, 0, Quat.LookDir(-Vec3.Forward));
		UI.WindowBegin("Bottom layout cut", ref windowPose, V.XY(0.0f, 0.1f));

		UI.LayoutPushCut(UICut.Bottom, 0.04f);
		UI.Button("Bottom");
		UI.LayoutPop();
		UI.Button("Top");

		UI.WindowEnd();
	}

	static Sprite img = null;
	static void PracticalTest()
	{
		Tests.Screenshot("Tests/LayoutCuts2.jpg", 1, 400, 400, 90, V.XYZ(0.5f, -0.17f, 0.22f), V.XYZ(0.5f, -0.17f, 0));

		if (img == null) img = Sprite.FromFile("floor.png");

		Pose windowPose = new Pose(0.5f, 0, 0, Quat.LookDir(-Vec3.Forward));
		UI.WindowBegin("Window Panel", ref windowPose);

		UI.LayoutPushCut(UICut.Top, UI.LineHeight);
		UI.Button  ("Back");
		UI.SameLine();
		UI.Text    ("Dagoth Wave", TextAlign.Center, TextFit.Clip, new Vec2(UI.LayoutRemaining.x, UI.LineHeight));
		UI.LayoutPop();

		float width = 0.1f;
		UI.LayoutPushCut(UICut.Left, width);
		UI.Image(img, new Vec2(width, 0));
		UI.LayoutPop();

		UI.LayoutReserve(0.2f, 0, false);
		UI.Text("Welcome Moon-and-Star\nCome to me through fire and war\n\nCome, Nerevar\nCome and look upon the heart, upon the heart\n\nLay down your weapons\nIt is not too late for my mercy\n\nCome and look upon the heart, upon the heart");

		UI.Button("Okay",  Vec2.Zero);
		UI.Button("N'wah", Vec2.Zero);

		UI.WindowEnd();
	}

	static void EmptyParentTest()
	{
		Tests.Screenshot("Tests/LayoutCuts3.jpg", 1, 400, 400, 90, V.XYZ(-0.4f, -0.04f, 0.10f), V.XYZ(-0.4f, -0.04f, 0));

		Pose windowPose = new Pose(-0.4f, 0, 0, Quat.LookDir(-Vec3.Forward));
		UI.WindowBegin("Empty", ref windowPose);
		UI.LayoutPushCut(UICut.Left, 0.1f);
		UI.LayoutReserve(Vec2.One * 0.05f);
		UI.HSeparator();
		UI.LayoutPop();
		UI.WindowEnd();
	}
}
