using System;
using StereoKit;

class TestUIPerf : ITest
{
	public void Step()
	{
		TestWindow("Buttons", V.XYZ( 0,   0,-0.5f), () => UI.Button("O"));
		TestWindow("Sliders", V.XYZ(-0.5f,0,-0.5f), () => { float v = 0.5f; UI.HSlider("O", ref v, 0, 1, 0, 0.01f ); });
		TestWindow("Inputs",  V.XYZ( 0.5f,0,-0.5f), () => { string v = "vvv"; UI.Input("O", ref v, V.XY(0.01f,0) ); });

		int s = 20;
		for (int i = 0; i < s*s; i++)
		{
			Pose pose = new Pose(V.XYZ((i%s) / (float)s - 0.5f, (int)(i/(float)s)/(float)s, -0.75f), Quat.LookDir(0, 0, 1));
			UI.PushId(i);
			UI.Handle("H", ref pose, new Bounds(Vec3.Zero, Vec3.One*0.01f), true);
			UI.PopId();
		}
	}

	static void TestWindow(string name, Vec3 pos, Action element)
	{
		Pose pose = new Pose(pos, Quat.LookDir(0, 0, 1));
		UI.WindowBegin(name, ref pose, new Vec2(0.4f, 0));
		for (int i = 0; i < 200; i++)
		{
			UI.PushId(i);
			element();
			UI.PopId();
			UI.SameLine();
		}
		UI.WindowEnd();
	}

	public void Initialize()
	{
	}

	public void Shutdown()
	{
	}
}