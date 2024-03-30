// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2024 Nick Klingensmith
// Copyright (c) 2023-2024 Qualcomm Technologies, Inc.

using StereoKit;

class DemoTextInput : ITest
{
	string title       = "Text Input";
	string description = "";
	Pose   windowPose  = Demo.contentPose.Pose;

	string text     = "Edit me";
	string textUri  = "https://stereokit.net";
	string number   = "1";
	string password = "123abc";

	public void Initialize() { }
	public void Shutdown  () { }

	public void Step()
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
		UI.Label("Password",     labelSize); UI.SameLine(); UI.Input("Password",   ref password, inputSize, TextContext.Password);

		UI.HSeparator();

		UI.Text("Soft keyboards don't show up when a physical keyboard is known to be handy! Flatscreen apps and sessions where the user has pressed physical keys recently won't show any soft keyboards.");

		bool forceKeyboard = Platform.ForceFallbackKeyboard;
		if(UI.Toggle("Fallback Keyboard", ref forceKeyboard))
			Platform.ForceFallbackKeyboard = forceKeyboard;

		UI.SameLine();
		bool openKeyboard = Platform.KeyboardVisible;
		if (UI.Toggle("Show Keyboard", ref openKeyboard))
			Platform.KeyboardShow(openKeyboard);

		UI.HSeparator();
		if (UI.Button("Switch Keyboard"))
		{
			Platform.KeyboardSetLayout(TextContext.Text, new string[]
			{
				"q|w|e|r|t|y|u|i|o|p\n---1|a|s|d|f|g|h|j|k|l\nspr:sk/ui/shift---3-visit_1|z|x|c|v|b|n|m|spr:sk/ui/backspace-\\b--3\nspr:sk/ui/close----close|123---3-go_2|,| - --7|.|Return-\\n--4|",
				"Q|W|E|R|T|Y|U|I|O|P\n---1|A|S|D|F|G|H|J|K|L\nspr:sk/ui/shift---3-go_0|Z|X|C|V|B|N|M|spr:sk/ui/backspace-\\b--3\nspr:sk/ui/close----close|123---3-go_2|!| - --7|?|Return-\\n--4|",
				"1|2|3|4|5|6|7|8|9|0\n---1|\\-|/|:|;|(|)|$|&|@\nspr:sk/ui/shift---3-go_0|*|=|+|#|%|'|\"|spr:sk/ui/backspace-\\b--3\nspr:sk/ui/close----close|123---3-go_0|!| - --7|?|Return-\\n--4|"
			});
		}

		UI.WindowEnd();

		Demo.ShowSummary(title, description, new Bounds(V.XY0(0, -0.19f), V.XYZ(.4f, .5f, 0)));
	}
}