using StereoKit;
using StereoKit.Framework;

class DemoSkeleton : ITest
{
	string title       = "Skeleton Estimation";
	string description = "With knowledge about where the head and hands are, you can make a decent guess about where some other parts of the body are! The StereoKit repository contains an AvatarSkeleton IStepper to show a basic example of how something like this can be done.";

	AvatarSkeleton avatar;
	public void Initialize()
	{
		avatar = SK.AddStepper<AvatarSkeleton>();
	}

	public void Step()
	{
		Demo.ShowSummary(title, description);
	}

	public void Shutdown()
	{ 
		SK.RemoveStepper(avatar);
	}
}