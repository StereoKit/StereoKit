using StereoKit;

internal class TestPanel : ITest
{
	public void Initialize()
	{
	}

	public void Shutdown()
	{
	}

	public void Update()
	{
		Pose p = new Pose(-.15f,0,-0.5f, Quat.LookDir(0,0,1));
		UI.WindowBegin("Panel Test Auto", ref p);

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

		p = new Pose(.15f, 0, -0.5f, Quat.LookDir(0, 0, 1));
		UI.WindowBegin("Panel Test Fixed", ref p, new Vec2(0.25f, 0));

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

		Tests.Screenshot("Tests/UIPanels.jpg", 600, 600, Vec3.Zero, new Vec3(0.0f, 0.0f, -0.5f));
	}
}