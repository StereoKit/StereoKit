using StereoKit;

class TestGerman : ITest
{
	public void Initialize() { }
	public void Shutdown() { }
	public void Update() {
		Text.Add(
			"Zwölf Boxkämpfer jagen Viktor quer über den großen Sylter Deich.",
			Matrix.TR(new Vec3(0, 0, 0), Quat.LookDir(0, 0, 1)),
			new Vec2(40,100)*U.cm, TextFit.Wrap);
		
		Tests.Screenshot("GermanText.jpg", 400, 400, new Vec3(0,0,.2f), Vec3.Zero);
	}
}