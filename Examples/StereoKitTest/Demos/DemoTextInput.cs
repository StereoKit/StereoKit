// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2024 Nick Klingensmith
// Copyright (c) 2023-2024 Qualcomm Technologies, Inc.

using StereoKit;
using System.Collections.Generic;
using System.Linq;

class DemoTextInput : ITest
{
	string title       = "Text Input";
	string description = "";

	public void Step()
	{
		ShowTextInputWindow();
		ShowRawInputWindow();

		Demo.ShowSummary(title, description, new Bounds(V.XY0(-.12f, -0.22f), V.XYZ(.6f, .56f, 0)));
	}

	Pose   uiWinPose = Pose.Identity;
	string text      = "Edit me";
	string textUri   = "https://stereokit.net";
	string number    = "1";
	string password  = "123abc";
	void ShowTextInputWindow()
	{
		UI.WindowBegin("Text Input", ref uiWinPose);

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
				"q|w|e|r|t|y|u|i|o|p\n---1|a|s|d|f|g|h|j|k|l\nspr:sk/ui/shift---3-visit_1|z|x|c|v|b|n|m|spr:sk/ui/backspace--8-3\nspr:sk/ui/close----close|123---3-go_2|,| - --7|.|Return-\\n--4|",
				"Q|W|E|R|T|Y|U|I|O|P\n---1|A|S|D|F|G|H|J|K|L\nspr:sk/ui/shift---3-go_0|Z|X|C|V|B|N|M|spr:sk/ui/backspace--8-3\nspr:sk/ui/close----close|123---3-go_2|!| - --7|?|Return-\\n--4|",
				"1|2|3|4|5|6|7|8|9|0\n---1|\\-|/|:|;|(|)|$|&|@\nspr:sk/ui/shift---3-go_0|*|=|+|#|%|'|\"|spr:sk/ui/backspace--8-3\nspr:sk/ui/close----close|123---3-go_0|!| - --7|?|Return-\\n--4|"
			});
		}

		UI.WindowEnd();
	}

	/// :CodeSample: Input.KeyboardEventCount Input.KeyboardEventAt
	/// ### Raw Keyboard Input
	// If you need to read the keyboard directly from a soft or hard keyboard,
	// this gives you the frame's events in the exact order the user made them.
	// Text events are language and keyboard layout sensitive, which makes them
	// the correct choice for text content, and key events carry the editing
	// intent that text can't express, like backspace and enter.
	//
	// `Input.KeyboardConsume` reads events destructively, so an element like
	// UI.Input can hide input from whatever comes after it. Reading by index
	// observes the frame's events without consuming anything, which is what a
	// display like this window wants.
	Pose         rawWinPose = new Pose(0.3f,0,0);
	List<string> uniChars   = new List<string>(Enumerable.Repeat("", 10));
	void ShowRawInputWindow()
	{
		UI.WindowBegin("Raw keyboard events:", ref rawWinPose);

		// Read each of this frame's events, even the ones an earlier UI.Input
		// may have consumed.
		for (int evt = 0; evt < Input.KeyboardEventCount; evt++)
		{
			KeyboardEvent e = Input.KeyboardEventAt(evt);
			// Text events carry their character data in e.Text, which handles
			// emoji and other codepoints too large for a single C# char.
			string desc = e.type == KeyboardEventType.Text
				? $"U+{char.ConvertToUtf32(e.Text, 0):X4} '{e.Text}'"
				: $"{e.key} {(e.type == KeyboardEventType.KeyPress ? "down" : "up")}";

			// Insert at the start of the list, and bump off any more than 10.
			uniChars.Insert(0, desc);
			if (uniChars.Count > 10)
				uniChars.RemoveAt(uniChars.Count - 1);
		}

		// Show each event as a label
		for (int i = 0; i < uniChars.Count; i++)
			UI.Label(uniChars[i]);

		UI.WindowEnd();
	}
	/// :End:

	public void Initialize()
	{
		rawWinPose = (Demo.contentPose * rawWinPose.ToMatrix()).Pose;
		uiWinPose  = (Demo.contentPose * uiWinPose .ToMatrix()).Pose;
	}
	public void Shutdown() { }
}