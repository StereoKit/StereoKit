using StereoKit;
using StereoKit.Framework;

class DemoSkeleton : ITest
{
	Matrix descPose    = Matrix.TR (-0.5f, 0, -0.5f, Quat.LookDir(1,0,1));
	string description = "With knowledge about where the head and hands are, you can make a decent guess about where some other parts of the body are! The StereoKit repository contains an AvatarSkeleton IStepper to show a basic example of how something like this can be done.";
	Matrix titlePose   = Matrix.TRS(V.XYZ(-0.5f, 0.05f, -0.5f), Quat.LookDir(1, 0, 1), 2);
	string title       = "Skeleton Estimation";

	AvatarSkeleton avatar;
	public void Initialize()
	{
		avatar = SK.AddStepper<AvatarSkeleton>();
	}

	public void Update()
	{
		Text.Add(title, titlePose);
		Text.Add(description, descPose, V.XY(0.4f, 0), TextFit.Wrap, TextAlign.TopCenter, TextAlign.TopLeft);
	}

	public void Shutdown()
	{ 
		SK.RemoveStepper(avatar);
	}
}