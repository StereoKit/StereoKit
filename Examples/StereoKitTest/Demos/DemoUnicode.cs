using StereoKit;

class DemoUnicode : ITest
{
	Model atlasModel;
	Model clipboard = Model.FromFile("Clipboard.glb");

	Pose  clipboardPose  = Pose.Identity;
	Pose  unicodeWinPose = Pose.Identity;

	string unicodeText = "";

	Font      japaneseFont;
	TextStyle japaneseStyle;

	Font      symbolFont;
	TextStyle symbolStyle;

	public void Initialize()
	{
		japaneseFont = Font.FromFile("C:/Windows/Fonts/YuGothR.ttc") ?? Font.Default;
		japaneseStyle = Text.MakeStyle(japaneseFont, 1*U.cm, Color.White);

		symbolFont  = Font.FromFile("C:/Windows/Fonts/segmdl2.ttf") ?? Font.Default;
		symbolStyle = Text.MakeStyle(symbolFont, 2*U.cm, Color.White);

		atlasModel = new Model(Mesh.Quad, TextStyle.Default.Material);
		atlasModel.SetTransform(0, Matrix.T(0,0,-0.01f));
	}

	public void Shutdown()
	{
	}

	public void Update()
	{
		
		UI.HandleBegin("", ref clipboardPose, clipboard.Bounds);
		clipboard.Draw(Matrix.Identity);
		UI.LayoutArea(V.XY0(12, 15) * U.cm, V.XY(24, 30) * U.cm);
		UI.Label("Font Atlas");
		UI.HSeparator();
		UI.Model(atlasModel);
		UI.HandleEnd();

		UI.WindowBegin("Unicode Samples", ref unicodeWinPose);
		UI.Label("Just type some Unicode here:");
		UI.Input("UnicodeText", ref unicodeText);
		UI.HSeparator();
		UI.PushTextStyle(japaneseStyle);
		UI.Text("古池や\n蛙飛び込む\n水の音\n- Matsuo Basho");
		UI.PopTextStyle();
		UI.HSeparator();
		UI.Text("Съешь же ещё этих мягких французских булок да выпей чаю. Широкая электрификация южных губерний даст мощный толчок подъёму сельского хозяйства. В чащах юга жил бы цитрус? Да, но фальшивый экземпляр!");
		UI.HSeparator();
		UI.PushTextStyle(symbolStyle);
		UI.Text("");
		UI.PopTextStyle();
		UI.WindowEnd();
	}
}
