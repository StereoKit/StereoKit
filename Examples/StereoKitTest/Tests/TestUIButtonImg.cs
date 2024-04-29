using StereoKit;

class TestUIButtonImg : ITest
{
	Sprite img = Sprite.FromFile("power.png");

	public void Initialize() => Tests.RunForFrames(2);
	public void Shutdown() { }

	public void Step()
	{
		Pose pose = new Pose(0,0,-0.5f, Quat.LookDir(0,0,1));
		UI.WindowBegin("Image Buttons", ref pose);

		UI.PushId(1);
		UI.ButtonImg("Left", img, UIBtnLayout.Left);
		UI.SameLine();
		UI.ButtonImg("Right", img, UIBtnLayout.Right);

		UI.ButtonImg("Center", img, UIBtnLayout.Center);
		UI.SameLine();
		UI.ButtonImg("NoText", img, UIBtnLayout.CenterNoText);

		UI.ButtonImg("None", img, UIBtnLayout.None);
		UI.PopId();


		UI.HSeparator();


		UI.PushId(2);
		Vec2 size = new Vec2(0.1f, UI.LineHeight * 2);
		UI.ButtonImg("Left", img, UIBtnLayout.Left, size);
		UI.SameLine();
		UI.ButtonImg("Right", img, UIBtnLayout.Right, size);

		UI.ButtonImg("Center", img, UIBtnLayout.Center, size);
		UI.SameLine();
		UI.ButtonImg("NoText", img, UIBtnLayout.CenterNoText, size);

		UI.ButtonImg("None", img, UIBtnLayout.None, size);
		UI.PopId();

		UI.WindowEnd();

		float y = UI.LayoutLast.center.y;
		Tests.Screenshot("UI/ButtonImg.jpg", 1, 400, 600, 90, V.XYZ(0, y, -0.3f), new Vec3(0, y, -0.5f));
	}
}