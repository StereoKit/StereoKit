using StereoKit;

class DemoVirtualKeyboard : ITest
{
	Pose windowPose = new Pose(-.4f, 0, 0, Quat.LookDir(1, 0, 1));
	string data = "Edit me";
	string number = "1";

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
		UI.Label("uri field");
		UI.SameLine();
		UI.Input("test Text uri", ref data,default,InputTextContextType.TextUri);
		UI.Label("number field Signed Decimal");
		UI.SameLine();
		UI.Input("Test number Signed Decimal", ref number, default, InputTextContextType.NumberSignedDecimal);
		UI.Label("number field Signed");
		UI.SameLine();
		UI.Input("Test number Signed", ref number, default, InputTextContextType.NumberSigned);
		UI.Label("number field");
		UI.SameLine();
		UI.Input("Test number", ref number, default, InputTextContextType.Number);
		var forceKeyboard = Platform.ForceVirtualKeyboard;
		if(UI.Toggle("Force Virtual Keyboard",ref forceKeyboard))
        {
			Platform.ForceVirtualKeyboard = forceKeyboard;
		}
		UI.SameLine();
		var oepnKeyboard = Platform.KeyboardVisible;
		if (UI.Toggle("Open Keyboard", ref oepnKeyboard))
		{
			Platform.KeyboardShow(oepnKeyboard,InputTextContextType.Text);
		}
		UI.WindowEnd();
	}

	public void Shutdown()
	{
	}
}