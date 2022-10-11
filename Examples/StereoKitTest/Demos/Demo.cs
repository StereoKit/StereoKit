using StereoKit;

abstract class Demo
{
	public static readonly Matrix contentPose = Matrix.TR(0.5f, 0, -0.4f, Quat.LookDir(-1, 0, 1));
	public static readonly Matrix descPose    = Matrix.TR (-0.5f, 0, -0.5f, Quat.LookDir(1,0,1));
	public static readonly Matrix titlePose   = Matrix.TRS(V.XYZ(-0.5f, 0.05f, -0.5f), Quat.LookDir(1, 0, 1), 2);

	public static void ShowSummary(string title, string description)
	{
		if (Tests.IsTesting)
		{
			Tests.Screenshot($"Demos/{title.Replace(" ", "")}.jpg", 1, 480, 270, 60, contentPose.Transform(V.XYZ(0,0.05f,-0.5f)), contentPose.Transform(V.XYZ(0,-0.1f,0)));
			return;
		}

		Text.Add(title,       titlePose);
		Text.Add(description, descPose, V.XY(0.4f, 0), TextFit.Wrap, TextAlign.TopCenter, TextAlign.TopLeft);
	}
}