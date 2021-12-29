using StereoKit;

class DemoVirtualKeyboard : ITest
{
	Pose windowPose = new Pose(-.4f, 0, 0, Quat.LookDir(1, 0, 1));
	string data = "Edit me";
	string number = "1";
	string noneNative = "Force none native Edit me";

	public void Initialize()
	{
	}

	public void Update()
    {
        UI.WindowBegin("Virtual Keyboard", ref windowPose);
		UI.Button("this button will take the keyboard focus");
		UI.PushKeyboardFocusSteal(false);
		UI.Button("this button will not");
		UI.PopKeyboardFocusSteal();
		UI.Input("testText", ref data);
		UI.Label("number field Signed Decimal");
		UI.SameLine();
		UI.Input("Test number Signed Decimal", ref number, default,true,KeyboardInputType.Number_Signed_Decimal);
		UI.Label("number field Signed");
		UI.SameLine();
		UI.Input("Test number Signed", ref number, default, true, KeyboardInputType.Number_Signed);
		UI.Label("number field");
		UI.SameLine();
		UI.Input("Test number", ref number, default, true, KeyboardInputType.Number);
		UI.Label("Force Virtual Keyboard");
		UI.SameLine();
		UI.Input("Test force virtual", ref noneNative, default, false);
		UI.WindowEnd();
	}

	public void Shutdown()
	{
	}
}