using StereoKit;

class DocFeatureImage : ITest
{
	Sprite logoSprite    = Sprite.FromFile("StereoKitWide.png", SpriteType.Single);
	Pose   clipboardPose = new Pose(.39f, -0.05f, -.55f, Quat.LookDir(-1, .4f, 1));
	float  clipSlider    = 0.75f;

	Model helmet     = Model.FromFile("DamagedHelmet.gltf");
	Pose  helmetPose = new Pose(.04f,-0.1f,-.55f, Quat.LookDir(1,.5f,1));

	Vec3 screenshotFrom = new Vec3(0.103f, -0.032f, -0.308f);
	Vec3 screenshotAt   = new Vec3(0.297f, -0.536f, -1.149f);

	// Oculus hand joints have a twisted thumb
	static void FixHand(HandJoint[] joints, Quat rot, Vec3 translate)
	{
		for (int i = 0; i < joints.Length; i++)
		{
			if (i<5) joints[i].orientation *= rot;
			joints[i].position += translate;
		}
	}

	public void Initialize()
	{
		Tests.RunForFrames(2);

		HandJoint[] leftHand  = new HandJoint[] { new HandJoint(V.XYZ(0.010f, -0.174f, -0.342f), new Quat(-0.098f, 0.648f, -0.010f, -0.755f), 0.020f), new HandJoint(V.XYZ(0.010f, -0.174f, -0.342f), new Quat(-0.098f, 0.648f, -0.010f, -0.755f), 0.020f), new HandJoint(V.XYZ(0.042f, -0.169f, -0.347f), new Quat(-0.096f, 0.646f, -0.119f, -0.748f), 0.012f), new HandJoint(V.XYZ(0.074f, -0.159f, -0.352f), new Quat(-0.104f, 0.649f, -0.016f, -0.753f), 0.010f), new HandJoint(V.XYZ(0.099f, -0.154f, -0.354f), new Quat(-0.104f, 0.649f, -0.016f, -0.753f), 0.009f), new HandJoint(V.XYZ(-0.001f, -0.171f, -0.348f), new Quat(-0.377f, 0.303f, -0.258f, -0.836f), 0.021f), new HandJoint(V.XYZ(0.019f, -0.121f, -0.376f), new Quat(-0.423f, 0.463f, -0.160f, -0.763f), 0.010f), new HandJoint(V.XYZ(0.041f, -0.091f, -0.384f), new Quat(-0.384f, 0.474f, -0.167f, -0.775f), 0.009f), new HandJoint(V.XYZ(0.056f, -0.072f, -0.390f), new Quat(-0.391f, 0.455f, -0.201f, -0.774f), 0.008f), new HandJoint(V.XYZ(0.067f, -0.054f, -0.396f), new Quat(-0.391f, 0.455f, -0.201f, -0.774f), 0.007f), new HandJoint(V.XYZ(-0.014f, -0.175f, -0.357f), new Quat(-0.377f, 0.303f, -0.258f, -0.836f), 0.021f), new HandJoint(V.XYZ(0.001f, -0.123f, -0.389f), new Quat(-0.406f, 0.318f, -0.268f, -0.814f), 0.011f), new HandJoint(V.XYZ(0.014f, -0.087f, -0.409f), new Quat(-0.328f, 0.342f, -0.250f, -0.844f), 0.008f), new HandJoint(V.XYZ(0.025f, -0.067f, -0.424f), new Quat(-0.324f, 0.351f, -0.290f, -0.829f), 0.008f), new HandJoint(V.XYZ(0.034f, -0.048f, -0.438f), new Quat(-0.324f, 0.351f, -0.290f, -0.829f), 0.007f), new HandJoint(V.XYZ(-0.028f, -0.177f, -0.369f), new Quat(-0.377f, 0.303f, -0.258f, -0.836f), 0.019f), new HandJoint(V.XYZ(-0.012f, -0.135f, -0.400f), new Quat(-0.389f, 0.239f, -0.342f, -0.821f), 0.010f), new HandJoint(V.XYZ(-0.007f, -0.103f, -0.423f), new Quat(-0.264f, 0.299f, -0.335f, -0.854f), 0.008f), new HandJoint(V.XYZ(0.002f, -0.086f, -0.442f), new Quat(-0.249f, 0.334f, -0.324f, -0.849f), 0.007f), new HandJoint(V.XYZ(0.010f, -0.069f, -0.458f), new Quat(-0.249f, 0.334f, -0.324f, -0.849f), 0.006f), new HandJoint(V.XYZ(-0.031f, -0.181f, -0.376f), new Quat(-0.251f, 0.259f, -0.470f, -0.805f), 0.018f), new HandJoint(V.XYZ(-0.023f, -0.151f, -0.410f), new Quat(-0.221f, 0.118f, -0.425f, -0.870f), 0.009f), new HandJoint(V.XYZ(-0.022f, -0.136f, -0.438f), new Quat(-0.114f, 0.135f, -0.461f, -0.870f), 0.007f), new HandJoint(V.XYZ(-0.020f, -0.130f, -0.457f), new Quat(-0.157f, 0.166f, -0.457f, -0.860f), 0.006f), new HandJoint(V.XYZ(-0.018f, -0.120f, -0.476f), new Quat(-0.157f, 0.166f, -0.457f, -0.860f), 0.005f), new HandJoint(V.XYZ(-0.016f, -0.161f, -0.363f), new Quat(0.324f, 0.397f, 0.032f, -0.858f), 0.000f), new HandJoint(V.XYZ(-0.033f, -0.198f, -0.337f), new Quat(-0.377f, 0.303f, -0.258f, -0.836f), 0.000f) };
		HandJoint[] rightHand = new HandJoint[] { new HandJoint(V.XYZ(0.322f, -0.290f, -0.345f), new Quat(0.099f, 0.353f, -0.126f, 0.922f), 0.020f), new HandJoint(V.XYZ(0.322f, -0.290f, -0.345f), new Quat(0.099f, 0.353f, -0.126f, 0.922f), 0.020f), new HandJoint(V.XYZ(0.301f, -0.281f, -0.369f), new Quat(0.085f, 0.139f, -0.237f, 0.958f), 0.012f), new HandJoint(V.XYZ(0.293f, -0.273f, -0.402f), new Quat(0.039f, 0.150f, -0.148f, 0.977f), 0.010f), new HandJoint(V.XYZ(0.286f, -0.269f, -0.425f), new Quat(0.039f, 0.150f, -0.148f, 0.977f), 0.009f), new HandJoint(V.XYZ(0.334f, -0.287f, -0.343f), new Quat(0.361f, 0.123f, -0.309f, 0.871f), 0.021f), new HandJoint(V.XYZ(0.332f, -0.240f, -0.378f), new Quat(0.330f, 0.031f, -0.311f, 0.891f), 0.010f), new HandJoint(V.XYZ(0.338f, -0.217f, -0.408f), new Quat(0.208f, 0.076f, -0.329f, 0.918f), 0.009f), new HandJoint(V.XYZ(0.338f, -0.206f, -0.430f), new Quat(0.197f, 0.056f, -0.349f, 0.914f), 0.008f), new HandJoint(V.XYZ(0.340f, -0.196f, -0.451f), new Quat(0.197f, 0.056f, -0.349f, 0.914f), 0.007f), new HandJoint(V.XYZ(0.349f, -0.294f, -0.347f), new Quat(0.361f, 0.123f, -0.309f, 0.871f), 0.021f), new HandJoint(V.XYZ(0.352f, -0.247f, -0.385f), new Quat(0.031f, 0.060f, -0.370f, 0.927f), 0.011f), new HandJoint(V.XYZ(0.349f, -0.243f, -0.428f), new Quat(-0.416f, 0.231f, -0.308f, 0.824f), 0.008f), new HandJoint(V.XYZ(0.331f, -0.258f, -0.443f), new Quat(-0.542f, 0.254f, -0.321f, 0.734f), 0.008f), new HandJoint(V.XYZ(0.313f, -0.274f, -0.452f), new Quat(-0.542f, 0.254f, -0.321f, 0.734f), 0.007f), new HandJoint(V.XYZ(0.363f, -0.300f, -0.351f), new Quat(0.361f, 0.123f, -0.309f, 0.871f), 0.019f), new HandJoint(V.XYZ(0.365f, -0.263f, -0.391f), new Quat(-0.079f, 0.086f, -0.417f, 0.901f), 0.010f), new HandJoint(V.XYZ(0.356f, -0.266f, -0.429f), new Quat(-0.450f, 0.260f, -0.379f, 0.765f), 0.008f), new HandJoint(V.XYZ(0.336f, -0.279f, -0.442f), new Quat(-0.530f, 0.322f, -0.366f, 0.694f), 0.007f), new HandJoint(V.XYZ(0.316f, -0.291f, -0.449f), new Quat(-0.530f, 0.322f, -0.366f, 0.694f), 0.006f), new HandJoint(V.XYZ(0.367f, -0.306f, -0.356f), new Quat(0.265f, 0.077f, -0.528f, 0.803f), 0.018f), new HandJoint(V.XYZ(0.374f, -0.283f, -0.395f), new Quat(-0.128f, 0.122f, -0.488f, 0.854f), 0.009f), new HandJoint(V.XYZ(0.364f, -0.286f, -0.424f), new Quat(-0.552f, 0.322f, -0.426f, 0.641f), 0.007f), new HandJoint(V.XYZ(0.346f, -0.295f, -0.428f), new Quat(-0.516f, 0.344f, -0.459f, 0.636f), 0.006f), new HandJoint(V.XYZ(0.326f, -0.302f, -0.434f), new Quat(-0.516f, 0.344f, -0.459f, 0.636f), 0.005f), new HandJoint(V.XYZ(0.353f, -0.281f, -0.351f), new Quat(-0.361f, 0.305f, -0.131f, 0.871f), 0.000f), new HandJoint(V.XYZ(0.355f, -0.315f, -0.316f), new Quat(0.361f, 0.123f, -0.309f, 0.871f), 0.000f) };
		Vec3 leftOff = (screenshotAt - screenshotFrom).Normalized * 0.06f;
		leftOff += Vec3.PerpendicularRight(leftOff, Vec3.Up).Normalized * -0.1f;

		FixHand(leftHand,  Quat.FromAngles(0, 0,-90), leftOff);
		FixHand(rightHand, Quat.FromAngles(0, 0, 90), Vec3.Zero);
		Tests.Hand(Handed.Right, rightHand);
		Tests.Hand(Handed.Left,  leftHand);
	}

	public void Shutdown()
	{
	}

	public void Update()
	{
		
		helmet.Draw(helmetPose.ToMatrix(.1f));

		UI.WindowBegin(" Welcome!", ref clipboardPose, V.XY(.26f,0));
		UI.Image(logoSprite, new Vec2(24, 0) * U.cm);

		UI.HSeparator();

		UI.Text("StereoKit is a C#, code-first, lightweight Mixed Reality engine. Welcome back to .NET! :)\n\nBuilt around an intuitive API, an MR friendly UI, MR interaction systems, and more!");

		UI.Space(UI.LineHeight*0.5f);

		UI.PanelBegin();
		UI.Button("Load File");
		UI.SameLine();
		UI.Label("DamagedHelmet.gltf");

		UI.Label("Scale:");
		UI.SameLine();
		UI.HSlider("Slide", ref clipSlider, 0, 1, 0, 0, UIConfirm.Pinch);
		UI.PanelEnd();

		UI.WindowEnd();

		Tests.Screenshot("FeatureImage.jpg", 1, 830, 480, 90, screenshotFrom, screenshotAt);
	}
}