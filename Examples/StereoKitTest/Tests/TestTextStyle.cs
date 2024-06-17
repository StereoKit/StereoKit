using StereoKit;

class TestTextStyle : ITest
{
	TextStyle style1, style2, style3, style4;
	bool      toggle = false;
	string    input = "Testing";
	string    jap_input = "హలో";

	public void Initialize()
	{
		Tests.RunForFrames(2);
		style1 = Text.MakeStyle(
			Font.FromFile("aileron_font.ttf") ?? Default.Font,
			2 * U.cm,
			Color.HSV(0,1,1));

		style2 = Text.MakeStyle(
			Font.FromFile("aileron_font.ttf") ?? Default.Font,
			1 * U.cm,
			Color.HSV(.33f,.5f,1));

		style3 = Text.MakeStyle(
			Font.FromFamily("NotoSansOsage") ?? Default.Font,
			2 * U.cm,
			Color.HSV(0, 1, 1));

		style4 = Text.MakeStyle(
			Font.FromFamily("Nirmala UI, Segoe UI") ?? Default.Font,
			2 * U.cm,
			Color.HSV(0, 1, 1));
	}
	public void Shutdown() { }
	public void Step()
	{
		Tests.Screenshot("Tests/TextStyle.jpg", 1, 400, 600, 90, V.XYZ(0, -0.15f, 0.25f), V.XYZ(0, -0.15f, 0));

		Pose pose = new Pose(Vec3.Zero, Quat.LookDir(0, 0, 1));
		UI.PushTextStyle(style1);
		UI.WindowBegin("Text Style", ref pose);

		UI.Label("Should be red");
		UI.HSeparator();

		UI.PushTextStyle(style2);
		UI.Text("Should also be in times new roman, or possibly the default font.");
		UI.Button("Boop here");
		UI.PopTextStyle();

		UI.Toggle("Toggle here", ref toggle);
		UI.Input("Input", ref input);
		UI.Radio("Radio", toggle);

		UI.PushTextStyle(style3);
		UI.Input("Input3", ref jap_input);
		UI.PopTextStyle();

		UI.PushTextStyle(style4);
		UI.Input("Input4", ref jap_input);
		UI.PopTextStyle();

		UI.WindowEnd();
		UI.PopTextStyle();
	}
}