using System;
using StereoKit;

class TestUIDisabled : ITest
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
	int controlsEnabled = 0;

	public void Initialize() { Tests.RunForFrames(2); }
	public void Shutdown() { }

	public void Step()
	{
		Tests.Screenshot("Tests/DisabledUI.jpg", 1, 600, 600, 90, V.XYZ(0, -0.20f, 0.15f), V.XYZ(0, -0.25f, 0));

		Pose pose = new Pose(-.2f, 0, -0.6f, Quat.LookDir(0, 0, 1));
		UI.WindowBegin("Text Style", ref pose);

		UI.PushEnabled(false);
		if (UI.Button("UI.Button"))
		{
			controlsEnabled++;
			Log.Err("Button was pressed.");
		}
		if (UI.ButtonImg("UI.ButtonImg", sprSearch, UIBtnLayout.Left))
		{
			controlsEnabled++;
			Log.Err("ButtonImg was pressed.");
		}
		if (UI.ButtonImg("UI.ButtonImgNoText", sprSearch, UIBtnLayout.CenterNoText))
		{
			controlsEnabled++;
			Log.Err("ButtonImgNoText was pressed.");
		}
		UI.Label("DisabledLabel");
		UI.Image(sprSearch, new Vec2(2, 0) * U.cm);
		if (UI.Toggle("UI.Toggle", ref disabledToggles[0]))
		{
			controlsEnabled++;
			Log.Err("Toggle was toggled.");
		}
		UI.SameLine();
		if (UI.Toggle("UI.CustomImageToggle", ref disabledToggles[2], sprToggleOff, sprToggleOn))
		{
			controlsEnabled++;
			Log.Err("CustomImageToggle was toggled.");
		}

		if (UI.Radio("UI.DisRadio", disabledRadio == 0))
		{
			disabledRadio = 0;
			controlsEnabled++;
			Log.Err("Radio was modified.");
		}
		UI.SameLine();
		if (UI.Radio("UI.DisRadio1", disabledRadio == 1))
		{
			disabledRadio = 1;
			controlsEnabled++;
			Log.Err("Radio was modified.");
		}
		UI.SameLine();
		if (UI.Radio("UI.DisRadio2", disabledRadio == 2))
		{
			disabledRadio = 2;
			controlsEnabled++;
			Log.Err("Radio was modified.");
		}
		if (UI.HSlider("Push", ref sliderPushValue, 0, 1, 0, 0.15f, UIConfirm.Push))
		{
			controlsEnabled++;
			Log.Err("Horizontal slider was modified.");
		}
		if (UI.Input("Input", ref textInput))
		{
			controlsEnabled++;
			Log.Err("Text Input field value modified.");
		}
		UI.HProgressBar(percent);
		UI.Text("UI.Text", TextAlign.TopLeft);
		if (UI.VSlider("UI.VSlider", ref vSliderVal, 0, 1, 0, 0.15f, UIConfirm.Push))
		{
			controlsEnabled++;
			Log.Err("Vertical slider was modified.");
		}

		UI.PopEnabled();

		if (UI.ButtonImg("Exit", powerSprite))
		{
			SK.Quit();
		}
		UI.WindowEnd();
	}
}