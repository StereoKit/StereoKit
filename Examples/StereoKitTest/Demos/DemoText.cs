// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

using StereoKit;

class DemoText : ITest
{
	string title       = "Text";
	string description = "";

	Align alignX      = Align.XLeft;
	Align alignY      = Align.YTop;
	Pose  alignWindow = (Demo.contentPose * Matrix.T(0,-0.3f,0)).Pose;

	/// :CodeSample: Text.MakeStyle Font.FromFile Text.Add
	/// ### Drawing text with and without a TextStyle
	/// ![Basic text]({{site.url}}/img/screenshots/BasicText.jpg)
	/// We can use a TextStyle object to control how text gets displayed!
	TextStyle style;
	/// :End:

	public void Initialize() {
		/// :CodeSample: Text.MakeStyle Font.FromFile Text.Add
		/// In initialization, we can create the style from a font, a size, 
		/// and a base color. Overloads for MakeStyle can allow you to 
		/// override the default font shader, or provide a specific Material.
		style = Text.MakeStyle(
			Font.FromFile("aileron_font.ttf"),
			2 * U.cm,
			Color.HSV(0.55f, 0.62f, 0.93f));
		/// :End:
	}

	public void Shutdown() { }

	public void Step()
	{

		UI.WindowBegin("Alignment", ref alignWindow);
		Vec2 size = new Vec2(8 * U.cm, UI.LineHeight);
		if (UI.Radio("Left"   , alignX == Align.XLeft  , size)) alignX = Align.XLeft;   UI.SameLine();
		if (UI.Radio("CenterX", alignX == Align.XCenter, size)) alignX = Align.XCenter; UI.SameLine();
		if (UI.Radio("Right"  , alignX == Align.XRight , size)) alignX = Align.XRight;
		if (UI.Radio("Top"    , alignY == Align.YTop   , size)) alignY = Align.YTop;    UI.SameLine();
		if (UI.Radio("CenterY", alignY == Align.YCenter, size)) alignY = Align.YCenter; UI.SameLine();
		if (UI.Radio("Bottom" , alignY == Align.YBottom, size)) alignY = Align.YBottom;
		UI.WindowEnd();

		Hierarchy.Push(Matrix.R(0, 180, 0) * Demo.contentPose);
		Hierarchy.Push(Matrix.T(0.1f,0,0));
		Text.Add("X Center", Matrix.TR(new Vec3(0,.1f, 0), Quat.LookDir(0,0,1)), Pivot.Center,      alignX | alignY);
		Text.Add("X Left",   Matrix.TR(new Vec3(0,.15f,0), Quat.LookDir(0,0,1)), Pivot.CenterLeft,  alignX | alignY);
		Text.Add("X Right",  Matrix.TR(new Vec3(0,.2f, 0), Quat.LookDir(0,0,1)), Pivot.CenterRight, alignX | alignY);
		Lines.Add(new Vec3(0,.05f,0), new Vec3(0,.25f,0), Color32.White, 0.001f);
		Hierarchy.Pop();

		Hierarchy.Push(Matrix.T(-0.1f,0,0));
		Text.Add("Y Center", Matrix.TR(new Vec3(0,.1f, 0), Quat.LookDir(0,0,1)), Pivot.YCenter|Pivot.XCenter, alignX | alignY);
		Lines.Add(new Vec3(-0.05f, .1f, 0), new Vec3(.05f, .1f, 0), Color32.White, 0.001f);

		Text.Add("Y Top",    Matrix.TR(new Vec3(0,.15f,0), Quat.LookDir(0,0,1)), Pivot.YTop|Pivot.XCenter, alignX | alignY);
		Lines.Add(new Vec3(-0.05f, .15f, 0), new Vec3(.05f, .15f, 0), Color32.White, 0.001f);

		Text.Add("Y Bottom", Matrix.TR(new Vec3(0,.2f, 0), Quat.LookDir(0,0,1)), Pivot.YBottom|Pivot.XCenter, alignX | alignY);
		Lines.Add(new Vec3(-0.05f,.2f, 0), new Vec3(.05f,.2f, 0), Color32.White, 0.001f);
		Hierarchy.Pop();

		Hierarchy.Push(Matrix.T(0, -0.1f, 0));
		Tests.Screenshot("BasicText.jpg", 600, 300, Hierarchy.ToWorld(new Vec3(0, 0, 0.09f)), Hierarchy.ToWorld(Vec3.Zero));

		/// :CodeSample: Text.MakeStyle Font.FromFile Text.Add
		/// Then it's pretty trivial to just draw some text on the screen! Just call
		/// Text.Add on update. If you don't have a TextStyle available, calling it 
		/// without one will just fall back on the default style.
		// Text with an explicit text style
		Text.Add(
			"Here's\nSome\nMulti-line\nText!!", 
			Matrix.TR(new Vec3(0.1f, 0, 0), Quat.LookDir(0, 0, 1)),
			style);
		// Text using the default text style
		Text.Add(
			"Here's\nSome\nMulti-line\nText!!", 
			Matrix.TR(new Vec3(-0.1f, 0, 0), Quat.LookDir(0, 0, 1)));
		/// :End:

		Hierarchy.Push(Matrix.T(0, -0.2f, 0));
		Text.Add(
			"Here's Some Multi-line Text!!",
			Matrix.TR(new Vec3(0, 0.0f, 0), Quat.LookDir(0, 0, 1)),
			new Vec2(SKMath.Cos(Time.Totalf)*10+11, 20) * U.cm,
			TextFit.Clip,
			style, Pivot.Center, alignX | alignY);
		Hierarchy.Pop();

		Hierarchy.Pop();

		Hierarchy.Pop();

		Demo.ShowSummary(title, description, new Bounds(V.XY0(0,-0.07f), V.XYZ(.4f, .68f, 0.1f)));
	}
}