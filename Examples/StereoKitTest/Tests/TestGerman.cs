using StereoKit;
using System;

class TestGerman : ITest
{
	public void Initialize() { }
	public void Shutdown() { }
	public void Update() {
		Text.Add(
			"Zwölf Boxkämpfer jagen Viktor quer über den großen Sylter Deich.",
			Matrix.TR(new Vec3(0, 0, 0), Quat.LookDir(0, 0, 1)),
			new Vec2(40,100)*U.cm, TextFit.Wrap);

		Tests.Screenshot(400,400, "GermanText.jpg", new Vec3(0,0,.2f), Vec3.Zero);
	}
}