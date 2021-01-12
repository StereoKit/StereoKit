using StereoKit;
using System.Numerics;

class DemoText : ITest
{
	TextAlign alignX      = TextAlign.XLeft;
	TextAlign alignY      = TextAlign.YTop;
	Pose      alignWindow = new Pose(0.4f, 0, 0, Quat.LookDir(-1, 0, 1));

	/// :CodeSample: Text.MakeStyle Font.FromFile Text.Add
	/// ### Drawing text with and without a TextStyle
	/// ![Basic text]({{site.url}}/img/screenshots/BasicText.jpg)
	/// We can use a TextStyle object to control how text gets displayed!
	TextStyle style;
	/// :End:

	public void Initialize() {
		/// :CodeSample: Text.MakeStyle Font.FromFile Text.Add
		/// In initialization, we can create the style from a font, a size, a 
		/// material, and a base color.
		style = Text.MakeStyle(
			Font.FromFile("C:/Windows/Fonts/Arial.ttf"), 
			2 * U.cm,
			Default.MaterialFont.Copy(), 
			Color.HSV(0.05f, 0.7f, 0.8f));
		/// :End:
	}

	public void Shutdown() { }

	public void Update()
	{
		Hierarchy.Push(Matrix.T(0, 0, -0.3f));

		UI.WindowBegin("Alignment", ref alignWindow, new Vector2(20,0) * U.cm);
		Vector2 size = new Vector2(5 * U.cm, UI.LineHeight);
		if (UI.Radio("Left"   , alignX == TextAlign.XLeft  , size)) alignX = TextAlign.XLeft;   UI.SameLine();
		if (UI.Radio("CenterX", alignX == TextAlign.XCenter, size)) alignX = TextAlign.XCenter; UI.SameLine();
		if (UI.Radio("Right"  , alignX == TextAlign.XRight , size)) alignX = TextAlign.XRight;
		if (UI.Radio("Top"    , alignY == TextAlign.YTop   , size)) alignY = TextAlign.YTop;    UI.SameLine();
		if (UI.Radio("CenterY", alignY == TextAlign.YCenter, size)) alignY = TextAlign.YCenter; UI.SameLine();
		if (UI.Radio("Bottom" , alignY == TextAlign.YBottom, size)) alignY = TextAlign.YBottom;
		UI.WindowEnd();

		Hierarchy.Push(Matrix.T(0.1f,0,0));
		Text.Add("X Center", Matrix.TR(new Vector3(0,.1f, 0), Quat.LookDir(0,0,1)), TextAlign.XCenter|TextAlign.YCenter, alignX | alignY);
		Text.Add("X Left",   Matrix.TR(new Vector3(0,.15f,0), Quat.LookDir(0,0,1)), TextAlign.XLeft  |TextAlign.YCenter, alignX | alignY);
		Text.Add("X Right",  Matrix.TR(new Vector3(0,.2f, 0), Quat.LookDir(0,0,1)), TextAlign.XRight |TextAlign.YCenter, alignX | alignY);
		Lines.Add(new Vector3(0,.05f,0), new Vector3(0,.25f,0), Color32.White, 0.001f);
		Hierarchy.Pop();

		Hierarchy.Push(Matrix.T(-0.1f,0,0));
		Text.Add("Y Center", Matrix.TR(new Vector3(0,.1f, 0), Quat.LookDir(0,0,1)), TextAlign.YCenter|TextAlign.XCenter, alignX | alignY);
		Lines.Add(new Vector3(-0.05f, .1f, 0), new Vector3(.05f, .1f, 0), Color32.White, 0.001f);

		Text.Add("Y Top",    Matrix.TR(new Vector3(0,.15f,0), Quat.LookDir(0,0,1)), TextAlign.YTop|TextAlign.XCenter, alignX | alignY);
		Lines.Add(new Vector3(-0.05f, .15f, 0), new Vector3(.05f, .15f, 0), Color32.White, 0.001f);

		Text.Add("Y Bottom", Matrix.TR(new Vector3(0,.2f, 0), Quat.LookDir(0,0,1)), TextAlign.YBottom|TextAlign.XCenter, alignX | alignY);
		Lines.Add(new Vector3(-0.05f,.2f, 0), new Vector3(.05f,.2f, 0), Color32.White, 0.001f);
		Hierarchy.Pop();

		Hierarchy.Push(Matrix.T(0, -0.1f, 0));
		if (Tests.IsTesting)
			Renderer.Screenshot(Hierarchy.ToWorld(new Vector3(0, 0, 0.09f)), Hierarchy.ToWorld(Vec3.Zero), 600, 300, "../../../docs/img/screenshots/BasicText.jpg");
		/// :CodeSample: Text.MakeStyle Font.FromFile Text.Add
		/// Then it's pretty trivial to just draw some text on the screen! Just call
		/// Text.Add on update. If you don't have a TextStyle available, calling it 
		/// without one will just fall back on the default style.
		// Text with an explicit text style
		Text.Add(
			"Here's\nSome\nMulti-line\nText!!", 
			Matrix.TR(new Vector3(0.1f, 0, 0), Quat.LookDir(0, 0, 1)),
			style);
		// Text using the default text style
		Text.Add(
			"Here's\nSome\nMulti-line\nText!!", 
			Matrix.TR(new Vector3(-0.1f, 0, 0), Quat.LookDir(0, 0, 1)));
		/// :End:

		Hierarchy.Push(Matrix.T(0, -0.2f, 0));
		Text.Add(
			"Here's Some Multi-line Text!!",
			Matrix.TR(new Vector3(0, 0.0f, 0), Quat.LookDir(0, 0, 1)),
			new Vector2(SKMath.Cos(Time.Totalf)*10+11, 20) * U.cm,
			TextFit.Clip,
			style, TextAlign.Center, alignX | alignY);
		Hierarchy.Pop();

		Hierarchy.Pop();

		Hierarchy.Pop();
	}
}