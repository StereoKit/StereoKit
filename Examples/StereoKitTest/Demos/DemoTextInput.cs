// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKit;

class DemoTextInput : ITest
{
	string title       = "Text Input";
	string description = "";
	string keyboardLayoutSmall = "`-`-192|1-1-49|2-2-50|3-3-51|4-4-52|5-5-53|6-6-54|7-7-55|8-8-56|9-9-57|0-0-48|\\--\\--189|=-=-187|spr:sk/ui/backspace-\\b-8-3|spr:sk/ui/close----close\nTab-\\t-9-3|w-w-87|q-q-81|e-e-69|r-r-82|t-t-84|y-y-89|u-u-85|i-i-73|o-o-79|p-p-80|[-[-219|]-]-221|\\\\-\\\\-220\nEnter-\\n-13-4|a-a-65|s-s-83|d-d-68|f-f-70|g-g-71|h-h-72|j-j-74|k-k-75|l-l-76|;-;-186|'-'-222|Enter-\\n-13-3\nspr:sk/ui/shift--16-5-visit_1|z-z-90|x-x-88|c-c-67|v-v-86|b-b-66|n-n-78|m-m-77|,-,-188|.-.-190|/-/-191|spr:sk/ui/shift--16-2-visit_1|spr:sk/ui/arrow_up--38\nCtrl--17-4-mod|Cmd--91-3|Alt--18-3-mod| - -32-9|Alt--18-3-mod|Ctrl--17-3-mod|spr:sk/ui/arrow_left--37|spr:sk/ui/arrow_down--40|spr:sk/ui/arrow_right--39|";
	string keyboardLayoutCaps = "~-~-192|!-!-49|@-@-50|#-#-51|$-$-52|%-%-53|^-^-54|&-&-55|*-*-56|(-(-57|)-)-48|_-_-189|+-+-187|spr:sk/ui/backspace-\\b-8-3|spr:sk/ui/close----close\nTab-\\t-9-3|W-W-87|Q-Q-81|E-E-69|R-R-82|T-T-84|Y-Y-89|U-U-85|I-I-73|O-O-79|P-P-80|{-{-219|}-}-221|\\|-\\|-220\nEnter-\\n-13-4|A-A-65|S-S-83|D-D-68|F-F-70|G-G-71|H-H-72|J-J-74|K-K-75|L-L-76|:-:-186|\"-\"-222|Enter-\\n-13-3\nspr:sk/ui/shift--16-5-go_0|Z-Z-90|X-X-88|C-C-67|V-V-86|B-B-66|N-N-78|M-M-77|<-<-188|>->-190|?-?-191|spr:sk/ui/shift--16-2-go_0|spr:sk/ui/arrow_up--38\nCtrl--17-4-mod|Cmd--91-3|Alt--18-3-mod| - -32-9|Alt--18-3-mod|Ctrl--17-3-mod|spr:sk/ui/arrow_left--37|spr:sk/ui/arrow_down--40|spr:sk/ui/arrow_right--39|";
	Pose windowPose = Demo.contentPose.Pose;
	string text    = "Edit me";
	string textUri = "https://stereokit.net";
	string number  = "1";

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
			string[] keyboardLayouts = { keyboardLayoutSmall, keyboardLayoutCaps };
			Platform.KeyboardSetLayout(TextContext.Text, keyboardLayouts);
		}

		UI.WindowEnd();

		Demo.ShowSummary(title, description, new Bounds(V.XY0(0, -0.19f), V.XYZ(.4f, .5f, 0)));
	}
}