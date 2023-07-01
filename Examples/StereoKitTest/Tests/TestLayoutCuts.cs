using StereoKit;
using System;
using System.Text.RegularExpressions;

class TestLayoutCuts : ITest
{
	public void Initialize() { Tests.RunForFrames(2); }
	public void Shutdown() { }

	public void Step()
	{
		Tests.Screenshot("Tests/LayoutCuts.jpg", 1, 600, 400, 90, V.XYZ(0, -0.11f, 0.18f), V.XYZ(0, -0.11f, 0));

		Pose windowPose = new Pose(-0.1f,0,0, Quat.LookDir(-Vec3.Forward));
		UI.WindowBegin("Left layout cut", ref windowPose);

		UI.LayoutPushCut(UICut.Left, 0.1f);
		UI.Button("Left");
		UI.LayoutPop();
		UI.Button("Right");

		UI.WindowEnd();

		windowPose = new Pose(-0.1f,-0.1f,0, Quat.LookDir(-Vec3.Forward));
		UI.WindowBegin("Right layout cut", ref windowPose, V.XY(0.2f, 0.0f));

		UI.LayoutPushCut(UICut.Right, 0.1f);
		UI.Button("Right");
		UI.LayoutPop();
		UI.Button("Left");

		UI.WindowEnd();

		windowPose = new Pose(0.1f, 0, 0, Quat.LookDir(-Vec3.Forward));
		UI.WindowBegin("Top layout cut", ref windowPose);

		UI.LayoutPushCut(UICut.Top, 0.04f);
		UI.Button("Top");
		UI.LayoutPop();
		UI.Button("Bottom");

		UI.WindowEnd();

		windowPose = new Pose(0.1f, -0.16f, 0, Quat.LookDir(-Vec3.Forward));
		UI.WindowBegin("Bottom layout cut", ref windowPose, V.XY(0.0f, 0.1f));

		UI.LayoutPushCut(UICut.Bottom, 0.04f);
		UI.Button("Bottom");
		UI.LayoutPop();
		UI.Button("Top");

		UI.WindowEnd();
	}
}
