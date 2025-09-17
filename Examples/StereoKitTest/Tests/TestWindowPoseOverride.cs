using StereoKit;

class TestWindowPoseOverride : ITest
{
	public void Initialize()
	{
		Tests.RunForFrames(2);
	}
	public void Shutdown()
	{
	}

	Pose modifiablePose = new Pose(.1f,0,-0.5f, Quat.LookDir(0,1,1));
	public void Step()
	{
		// With this window, we're adding a constraint to a normal stateful
		// pose. This is a common scenario, where the developer wants a gravity
		// or floor oriented Handle/Window, like some sort of house plant
		// hologram. We do this by quashing out the X and Z rotation of the
		// Pose's orientation, and renormalizing it to make it a valid rotation.
		UI.WindowBegin("Modified Pose", ref modifiablePose);
		Hierarchy.Pop();
		modifiablePose.orientation.x = 0;
		modifiablePose.orientation.z = 0;
		modifiablePose.orientation.Normalize();
		Hierarchy.Push(modifiablePose);
		UI.Text("We're quashing rotation of this Window's pose on the X and Z axes after doing regular handle logic.");
		UI.WindowEnd();

		// With this window, we're completely overriding the pose. Detection of
		// interaction always happens with the initial pose (it has to), but
		// rendering happens completely using the override pose. This means
		// that this window will still be interactive from its _original_
		// location, despite not being visible there! This is an unusual
		// circumstance and not something we need to fix, usually the final
		// pose will be fed back in as the initial pose on the start of the
		// next frame.
		Pose pose = new Pose(-.1f, 1, -0.5f, Quat.LookDir(1,0,0));
		UI.WindowBegin("Override Pose", ref pose);
		Hierarchy.Pop();
		Hierarchy.Push(new Pose(-.1f,0,-0.5f, Quat.LookDir(0,0,1)));
		UI.Text("We're completely overriding this Window's pose to prove that even extreme modifications to the Window's pose will still properly apply.");
		UI.WindowEnd();


		Tests.Screenshot("UI/PoseOverride.jpg", 1, 400, 400, 40, new Vec3(0,-0.05f,0), new Vec3(0,-0.05f,-0.5f));
	}
}