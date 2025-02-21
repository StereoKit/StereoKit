using StereoKit;

internal class TestUIPanel : ITest
{
	public void Initialize()
	{
		Tests.RunForFrames(2);
	}

	public void Shutdown()
	{
	}

	Pose p1 = new Pose(-.15f,0,-0.5f, Quat.LookDir(0,0,1));
	Pose p2 = new Pose(.15f,0,-0.5f, Quat.LookDir(0, 0, 1));
	public void Step()
	{
		UI.WindowBegin("Panel Test Auto", ref p1);

		UI.PanelBegin(UIPad.None);
		UI.Button("UIPad.None");
		UI.HSeparator();
		UI.PanelEnd();

		UI.PanelBegin(UIPad.Inside);
		UI.Button("UIPad.Inside");
		UI.HSeparator();
		UI.PanelEnd();

		UI.PanelBegin(UIPad.Outside);
		UI.Button("UIPad.Outside");
		UI.HSeparator();
		UI.PanelEnd();

		UI.WindowEnd();


		UI.WindowBegin("Panel Test Fixed", ref p2, new Vec2(0.25f, 0));

		UI.PanelBegin(UIPad.None);
		UI.Button("UIPad.None");
		UI.HSeparator();
		UI.PanelEnd();

		UI.PanelBegin(UIPad.Inside);
		UI.Button("UIPad.Inside");
		UI.HSeparator();
		UI.PanelEnd();

		UI.PanelBegin(UIPad.Outside);
		UI.Button("UIPad.Outside");
		UI.HSeparator();
		UI.PanelEnd();

		UI.WindowEnd();

		Vec3 at = new Vec3(0.05f,-0.06f, -0.3f);
		Tests.Screenshot("UI/Panels.jpg", 1, 600, 400, 90, at, at - Vec3.UnitZ);
	}
}