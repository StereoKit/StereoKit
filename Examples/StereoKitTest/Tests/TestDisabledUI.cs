using StereoKit;

class TestDisabledUI : ITest
{
	Sprite sprSearch = Sprite.FromFile("search.png");
	Sprite sprToggleOn = Sprite.FromFile("mic_icon.png");
	Sprite sprToggleOff = Sprite.FromFile("power.png");
	Sprite powerSprite = Sprite.FromFile("power.png", SpriteType.Single);
	float sliderPushValue = 0;
	bool[] disabledToggles = new bool[5];
	int disabledRadio = 0;
	string textInput = "Text here please :)";
	float vSliderVal = 0;
	float percent = 10;

	public void Initialize()
	{
		Tests.RunForFrames(2);
	}
	public void Shutdown() { }
	public void Step()
	{
		Tests.Screenshot("Tests/DisabledUI.jpg", 1, 600, 600, 90, V.XYZ(0, -0.20f, 0.35f), V.XYZ(0, -0.25f, 0));

		Pose pose = new Pose(Vec3.Zero, Quat.LookDir(0, 0, 1));
		UI.WindowBegin("Text Style", ref pose);

		UI.PushEnabled(false);
		if (UI.Button("UI.Button"))
		{
			Log.Info("Button was pressed.");
		}
		if (UI.ButtonImg("UI.ButtonImg", sprSearch, UIBtnLayout.Left))
		{
			Log.Info("ButtonImg was pressed.");
		}
		if (UI.ButtonImg("UI.ButtonImgNoText", sprSearch, UIBtnLayout.CenterNoText))
		{
			Log.Info("ButtonImgNoText was pressed.");
		}
		UI.Label("DisabledLabel");
		UI.Image(sprSearch, new Vec2(2, 0) * U.cm);
		UI.Toggle("UI.Toggle", ref disabledToggles[0]);
		UI.SameLine();
		UI.Toggle("UI.CustomImageToggle", ref disabledToggles[2], sprToggleOff, sprToggleOn);

		if (UI.Radio("UI.DisRadio", disabledRadio == 0))
		{
			disabledRadio = 0;
		}
		UI.SameLine();
		if (UI.Radio("UI.DisRadio1", disabledRadio == 1))
		{
			disabledRadio = 1;
		}
		UI.SameLine();
		if (UI.Radio("UI.DisRadio2", disabledRadio == 2))
		{
			disabledRadio = 2;
		}
		UI.HSlider("Push", ref sliderPushValue, 0, 1, 0, 0.15f, UIConfirm.Push);
		UI.Input("Input", ref textInput);
		UI.ProgressBar(percent);
		UI.Text("UI.Text", TextAlign.TopLeft);
		UI.VSlider("UI.VSlider", ref vSliderVal, 0, 1, 0, 0, UIConfirm.Push);

		UI.PopEnabled();

		if (UI.ButtonImg("Exit", powerSprite))
		{
			SK.Quit();
		}
		UI.WindowEnd();
	}
}