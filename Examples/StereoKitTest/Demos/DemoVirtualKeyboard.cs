using StereoKit;

class DemoVirtualKeyboard : ITest
{
	Pose windowPose = new Pose(-.4f, 0, 0, Quat.LookDir(1, 0, 1));
	string data = "Edit me";
	KeyboardLayouts keyboardLayouta;
	public void Initialize()
	{
		keyboardLayouta = KeyboardLayouts.Default;
		keyboardLayouta.NormalKeyboard.Keys[0].Key = "a";
		keyboardLayouta.ShiftKeyboard.Keys[0].Key = "A";
		keyboardLayouta.NormalKeyboard.Keys[0].Width = 1;
		keyboardLayouta.ShiftKeyboard.Keys[0].Width = 1;
	}

	public void Update()
    {
        UI.WindowBegin("Virtual Keyboard", ref windowPose);
		if(UI.Button("Set Keyboard Layout to US"))
        {
			Input.VirtualKeyboard.KeyboardLayout = KeyboardLayouts.USKeyboardLayout;
		}
		UI.SameLine();
		if (UI.Button("Set Keyboard Layout to A"))
		{
			Input.VirtualKeyboard.KeyboardLayout = keyboardLayouta;
		}
		UI.Button("this button will take the keyboard focus");
		UI.PushKeyboardFocusSteal(false);
		UI.Button("this button will not");
		UI.PopKeyboardFocusSteal();
		UI.Input("testText", ref data);
		var open = Input.VirtualKeyboard.Open;
		UI.Toggle("Open", ref open);
		Input.VirtualKeyboard.Open = open;
		UI.SameLine();
		var shift = Input.VirtualKeyboard.Shift;
		UI.Toggle("Shift", ref shift);
		Input.VirtualKeyboard.Shift = shift;
		UI.WindowEnd();
	}

	public void Shutdown()
	{
	}
}