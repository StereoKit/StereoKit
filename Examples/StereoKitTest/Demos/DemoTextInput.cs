using StereoKit;

class DemoTextInput : ITest
{
	Pose windowPose = new Pose(0.5f, 0, -0.5f, Quat.LookDir(-1, 0, 1));
	string text    = "Edit me";
	string textUri = "https://stereokit.net";
	string number  = "1";

	public void Initialize() { }
	public void Shutdown  () { }

	public void Update()
	{
		UI.WindowBegin("Text Input", ref windowPose);

		UI.Text("You can specify whether or not a UI element will hide an active soft keyboard upon interaction.");
		UI.Button("Hides active soft keyboard");
		UI.SameLine();
		UI.PushPreserveKeyboard(true);
		UI.Button("Doesn't hide");
		UI.PopPreserveKeyboard();

		UI.HSeparator();

		UI.Text("Different TextContexts will surface different soft keyboards.");
		Vec2 inputSize = V.XY(20*U.cm,0);
		Vec2 labelSize = V.XY(8*U.cm,0);
		UI.Label("Normal Text",  labelSize); UI.SameLine(); UI.Input("TextText",    ref text,    inputSize, TextContext.Text);
		UI.Label("URI Text",     labelSize); UI.SameLine(); UI.Input("URIText",     ref textUri, inputSize, TextContext.Uri);
		UI.Label("Numeric Text", labelSize); UI.SameLine(); UI.Input("NumericText", ref number,  inputSize, TextContext.Number);

		UI.HSeparator();

		UI.Text("Soft keyboards don't show up when a physical keyboard is known to be handy! Flatscreen apps and sessions where the user has pressed physical keys recently won't show any soft keyboards.");

		bool forceKeyboard = Platform.ForceFallbackKeyboard;
		if(UI.Toggle("Fallback Keyboard", ref forceKeyboard))
			Platform.ForceFallbackKeyboard = forceKeyboard;

		UI.SameLine();
		bool openKeyboard = Platform.KeyboardVisible;
		if (UI.Toggle("Show Keyboard", ref openKeyboard))
			Platform.KeyboardShow(openKeyboard);

		UI.WindowEnd();
	}


}