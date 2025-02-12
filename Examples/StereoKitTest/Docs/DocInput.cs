using StereoKit;

class DocInput : ITest
{
	static void FingerGlowWindow()
	{
		Pose pose = new Pose(0,0,-0.5f, Quat.LookDir(0,0,1));
		UI.WindowBegin("Finger Glow", ref pose);
		UI.LayoutReserve(V.XX(0.1f));
		UI.WindowEnd();

		/// :CodeSample: Input.FingerGlow
		/// ### Disabling Finger Glow
		/// When using StereoKit's built-in UI shaders, or the shader API's
		/// `sk_finger_glow`, StereoKit provides a glowing aura around the
		/// pointer finger.
		/// 
		/// ![Finger Glow on a Window panel]({{site.screen_url}}/Docs/Input_FingerGlow.jpg)
		/// 
		/// This feature is on by default, but can be disabled without
		/// modifying shaders! As long as `Input.FingerGlow` is `false` at
		/// _the end of the frame_, StereoKit will skip providing the shaders
		/// with valid finger pose data for the glow effect.
		Input.FingerGlow = false;
		/// :End:
		
		// Our screenshot still needs this to be on
		Input.FingerGlow = true;
	}

	public void Initialize() => Tests.RunForFrames(2);
	public void Shutdown  () { }
	public void Step      ()
	{
		FingerGlowWindow();

		Tests.Hand(new HandJoint[]{ new HandJoint(V.XYZ(-0.009f, -0.171f, -0.402f), new Quat(0.221f, -0.047f, -0.827f, 0.515f), 0.020f), new HandJoint(V.XYZ(-0.009f, -0.171f, -0.402f), new Quat(0.221f, -0.047f, -0.827f, 0.515f), 0.020f), new HandJoint(V.XYZ(0.005f, -0.166f, -0.432f), new Quat(0.245f, 0.010f, -0.766f, 0.594f), 0.013f), new HandJoint(V.XYZ(0.018f, -0.155f, -0.463f), new Quat(0.221f, -0.045f, -0.827f, 0.516f), 0.010f), new HandJoint(V.XYZ(0.030f, -0.151f, -0.485f), new Quat(0.221f, -0.045f, -0.827f, 0.516f), 0.009f), new HandJoint(V.XYZ(-0.016f, -0.164f, -0.394f), new Quat(0.488f, -0.042f, -0.077f, 0.868f), 0.022f), new HandJoint(V.XYZ(-0.001f, -0.112f, -0.421f), new Quat(0.422f, 0.007f, -0.091f, 0.902f), 0.011f), new HandJoint(V.XYZ(0.002f, -0.082f, -0.446f), new Quat(0.338f, 0.008f, -0.065f, 0.939f), 0.009f), new HandJoint(V.XYZ(0.003f, -0.066f, -0.466f), new Quat(0.308f, 0.029f, -0.040f, 0.950f), 0.008f), new HandJoint(V.XYZ(0.002f, -0.051f, -0.484f), new Quat(0.308f, 0.029f, -0.040f, 0.950f), 0.007f), new HandJoint(V.XYZ(-0.033f, -0.161f, -0.390f), new Quat(0.488f, -0.042f, -0.077f, 0.868f), 0.022f), new HandJoint(V.XYZ(-0.023f, -0.106f, -0.417f), new Quat(0.276f, 0.070f, 0.005f, 0.958f), 0.012f), new HandJoint(V.XYZ(-0.029f, -0.082f, -0.454f), new Quat(-0.219f, 0.054f, 0.050f, 0.973f), 0.008f), new HandJoint(V.XYZ(-0.031f, -0.094f, -0.479f), new Quat(-0.497f, 0.040f, 0.136f, 0.856f), 0.008f), new HandJoint(V.XYZ(-0.030f, -0.116f, -0.493f), new Quat(-0.497f, 0.040f, 0.136f, 0.856f), 0.007f), new HandJoint(V.XYZ(-0.049f, -0.157f, -0.387f), new Quat(0.488f, -0.042f, -0.077f, 0.868f), 0.020f), new HandJoint(V.XYZ(-0.044f, -0.110f, -0.416f), new Quat(0.308f, 0.064f, 0.052f, 0.948f), 0.010f), new HandJoint(V.XYZ(-0.050f, -0.087f, -0.449f), new Quat(-0.235f, -0.000f, 0.113f, 0.965f), 0.008f), new HandJoint(V.XYZ(-0.048f, -0.099f, -0.473f), new Quat(-0.560f, -0.064f, 0.133f, 0.815f), 0.007f), new HandJoint(V.XYZ(-0.042f, -0.122f, -0.484f), new Quat(-0.560f, -0.064f, 0.133f, 0.815f), 0.006f), new HandJoint(V.XYZ(-0.058f, -0.158f, -0.389f), new Quat(0.459f, -0.018f, 0.173f, 0.871f), 0.019f), new HandJoint(V.XYZ(-0.064f, -0.120f, -0.417f), new Quat(0.381f, 0.037f, 0.111f, 0.917f), 0.009f), new HandJoint(V.XYZ(-0.069f, -0.098f, -0.439f), new Quat(-0.135f, -0.038f, 0.192f, 0.971f), 0.007f), new HandJoint(V.XYZ(-0.066f, -0.104f, -0.459f), new Quat(-0.482f, -0.152f, 0.167f, 0.846f), 0.007f), new HandJoint(V.XYZ(-0.057f, -0.120f, -0.472f), new Quat(-0.482f, -0.152f, 0.167f, 0.846f), 0.006f), new HandJoint(V.XYZ(-0.028f, -0.133f, -0.403f), new Quat(-0.269f, 0.025f, -0.084f, 0.959f), 0.000f), new HandJoint(V.XYZ(-0.039f, -0.187f, -0.363f), new Quat(0.488f, -0.042f, -0.077f, 0.868f), 0.000f) });
		Tests.Screenshot("Docs/Input_FingerGlow.jpg", 1, 400, 400, 90, V.XYZ(0, -0.05f, -0.3f), V.XYZ(0, -0.05f, -0.5f));
	}
}