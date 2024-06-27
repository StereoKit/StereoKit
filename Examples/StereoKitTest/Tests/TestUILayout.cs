using StereoKit;

class TestUILayout : ITest
{
	public void Initialize() { Tests.RunForFrames(2); }
	public void Shutdown() { }
	public void Step()
	{
		Tests.Screenshot("Tests/Layout.jpg", 1, 400, 600, 70, V.XYZ(0, -0.12f, 0.25f), V.XYZ(0, -0.12f, 0));

		Pose pose = new Pose(Vec3.Zero, Quat.LookDir(0,0,1));
		UI.WindowBegin("Layout", ref pose);

		Bounds layout = UI.LayoutReserve(Vec2.Zero, false, U.cm);
		Mesh.Cube.Draw(Material.Default, Matrix.TS(layout.center, layout.dimensions));
		Lines.Add(UI.LayoutAt, UI.LayoutAt + V.XYZ( 0,0,-U.cm * 3 ), Color32.Black, 0.004f);

		layout = UI.LayoutReserve(Vec2.Zero, false, U.cm);
		Mesh.Cube.Draw(Material.Default, Matrix.TS(layout.center, layout.dimensions));


		layout = UI.LayoutReserve(V.XY(4 * U.cm, 0), false, U.cm);
		Mesh.Cube.Draw(Material.Default, Matrix.TS(layout.center, layout.dimensions));

		UI.SameLine();
		UI.LayoutReserve(V.XY(4 * U.cm, 0), false, U.cm);
		layout = UI.LayoutLast;
		layout.center    .z -= U.cm / 2;
		layout.dimensions.z  = U.cm;
		Mesh.Cube.Draw(Material.Default, Matrix.TS(layout.center, layout.dimensions));


		UI.LayoutReserve(V.XY(4 * U.cm, 0), true);
		layout = UI.LayoutLast;
		layout.center    .z -= U.cm / 2;
		layout.dimensions.z  = U.cm;
		Mesh.Cube.Draw(Material.Default, Matrix.TS(layout.center, layout.dimensions));

		UI.SameLine();
		layout = UI.LayoutReserve(V.XY(4 * U.cm, 0), true, U.cm);
		Mesh.Cube.Draw(Material.Default, Matrix.TS(layout.center, layout.dimensions));


		UI.Button("hi", V.XY( 4 * U.cm, UI.LineHeight ));
		UI.SameLine();
		UI.Button("hi2", V.XY( 4 * U.cm, UI.LineHeight ));

		UI.Button("hi3");
		UI.SameLine();
		Lines.Add(UI.LayoutAt, UI.LayoutAt + V.XYZ(0, 0, -U.cm * 3), Color32.Black, 0.004f);
		UI.Button("hi4");

		UI.WindowEnd();

		Tests.Screenshot("Tests/LayoutSpace.jpg", 1, 500, 400, 70, V.XYZ(0.3f, -0.13f, 0.25f), V.XYZ(0.3f, -0.13f, 0));

		pose = new Pose(V.XY0(0.3f,0), Quat.LookDir(0, 0, 1));
		UI.WindowBegin("Space", ref pose);

		float space = 4 * U.cm;

		UI.LayoutPushCut(UICut.Left, 0.12f);

		UI.Label("HSpace at start");
		UI.PanelBegin();
		UI.HSpace(space);
		EmptyItem();
		UI.PanelEnd();

		UI.Label("VSpace at start");
		UI.PanelBegin();
		UI.VSpace(space);
		EmptyItem();
		UI.PanelEnd();

		UI.LayoutPop();

		UI.Label("HSpace after item");
		UI.PanelBegin();
		EmptyItem();
		UI.SameLine();
		UI.HSpace(space);
		EmptyItem();
		UI.SameLine();
		EmptyItem();
		EmptyItem();
		UI.PanelEnd();

		UI.Label("VSpace after item");
		UI.PanelBegin();
		EmptyItem();
		UI.SameLine();
		UI.VSpace(space);
		EmptyItem();
		UI.SameLine();
		EmptyItem();
		EmptyItem();
		UI.PanelEnd();

		UI.WindowEnd();
	}

	static void EmptyItem()
	{
		Bounds layout = UI.LayoutReserve(V.XY(4 * U.cm, 0), false, U.cm);
		Mesh.Cube.Draw(Material.Default, Matrix.TS(layout.center, layout.dimensions));
	}
}
