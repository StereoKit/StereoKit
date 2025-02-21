using StereoKit;

internal class DemoHandPoses : ITest
{
	string title       = "Hand Sim Poses";
	string description = "StereoKit simulates hand joints for controllers and mice, but sometimes you really just need to test a funky gesture!\n\nThe Input.HandSimPose functions allow you to customize how StereoKit simulates these hand poses, and this scene is a small tool to help you with capturing poses for these functions!";

	enum Mode
	{
		Rotate,
		Translate,
	}
	
	Pose windowPose = Demo.contentPose.Pose;

	bool   captured   = false;
	float  offRotX    = 0;
	float  offRotY    = 0;
	float  offRotZ    = 0;
	float  offPosX    = 0;
	float  offPosY    = 0;
	float  offPosZ    = 0;
	Mode   offMode    = Mode.Rotate;
	Pose[] handPose   = new Pose[] { new Pose(-0.020f, -0.026f, -0.039f, new Quat(0.247f, -0.374f, -0.725f, -0.523f)), new Pose(-0.020f, -0.026f, -0.039f, new Quat(0.247f, -0.374f, -0.725f, -0.523f)), new Pose(-0.021f, -0.053f, -0.059f, new Quat(0.373f, -0.123f, -0.764f, -0.512f)), new Pose(-0.006f, -0.072f, -0.083f, new Quat(0.466f, -0.071f, -0.792f, -0.388f)), new Pose(0.011f, -0.085f, -0.098f, new Quat(0.466f, -0.071f, -0.792f, -0.388f)), new Pose(-0.015f, -0.015f, -0.039f, new Quat(0.000f, -0.000f, 0.000f, -1.000f)), new Pose(-0.024f, -0.007f, -0.098f, new Quat(0.070f, -0.046f, -0.032f, -0.996f)), new Pose(-0.027f, -0.013f, -0.136f, new Quat(0.104f, -0.037f, -0.008f, -0.994f)), new Pose(-0.029f, -0.018f, -0.160f, new Quat(0.037f, -0.009f, 0.008f, -0.999f)), new Pose(-0.029f, -0.019f, -0.183f, new Quat(0.037f, -0.009f, 0.008f, -0.999f)), new Pose(0.002f, -0.010f, -0.037f, new Quat(0.000f, -0.000f, 0.000f, -1.000f)), new Pose(-0.002f, -0.003f, -0.098f, new Quat(0.491f, 0.062f, 0.039f, -0.868f)), new Pose(0.001f, -0.040f, -0.120f, new Quat(0.924f, 0.027f, 0.082f, -0.372f)), new Pose(-0.002f, -0.060f, -0.100f, new Quat(0.989f, 0.062f, 0.125f, -0.050f)), new Pose(-0.008f, -0.064f, -0.075f, new Quat(0.989f, 0.062f, 0.125f, -0.050f)), new Pose(0.015f, -0.006f, -0.035f, new Quat(0.000f, -0.000f, 0.000f, -1.000f)), new Pose(0.018f, -0.007f, -0.090f, new Quat(0.485f, 0.108f, 0.109f, -0.861f)), new Pose(0.021f, -0.041f, -0.111f, new Quat(0.847f, 0.046f, 0.192f, -0.494f)), new Pose(0.013f, -0.064f, -0.099f, new Quat(0.946f, -0.010f, 0.230f, -0.229f)), new Pose(0.003f, -0.076f, -0.080f, new Quat(0.946f, -0.010f, 0.230f, -0.229f)), new Pose(0.023f, -0.010f, -0.035f, new Quat(0.018f, 0.140f, 0.207f, -0.968f)), new Pose(0.036f, -0.014f, -0.079f, new Quat(0.102f, 0.241f, 0.128f, -0.957f)), new Pose(0.049f, -0.022f, -0.107f, new Quat(0.085f, 0.287f, 0.156f, -0.941f)), new Pose(0.060f, -0.027f, -0.124f, new Quat(0.055f, 0.244f, 0.153f, -0.956f)), new Pose(0.070f, -0.030f, -0.143f, new Quat(0.055f, 0.244f, 0.153f, -0.956f)) };
	HandSimId x1      = HandSimId.None;
	HandSimId x2      = HandSimId.None;
	HandSimId stick   = HandSimId.None;
	HandSimId menu    = HandSimId.None;
	HandSimId grip    = HandSimId.None;
	HandSimId trigger = HandSimId.None;
	HandSimId space   = HandSimId.None;


	public void Initialize()
	{
	}

	public void Shutdown()
	{
		Input.HandSimPoseRemove(x1);
		Input.HandSimPoseRemove(x2);
		Input.HandSimPoseRemove(stick);
		Input.HandSimPoseRemove(menu);
		Input.HandSimPoseRemove(grip);
		Input.HandSimPoseRemove(trigger);
		Input.HandSimPoseRemove(space);
	}

	public void Step()
	{
		UI.WindowBegin("Hand Pose Editor", ref windowPose, V.XY(0.35f, 0.0f));

		// Draw the hand preview next to the window
		if (!captured) handPose = CaptureHand();
		Matrix handPreviewRoot = Matrix.T(
			UI.LayoutAt.x - (UI.LayoutRemaining.x + 0.1f),
			UI.LayoutAt.y - 0.1f,
			0);
		DrawHandPose(handPreviewRoot, V.XYZ(offPosX, offPosY, offPosZ), V.XYZ(offRotX, offRotY, offRotZ), handPose, captured);

		// UI for assigning the hand pose to inputs
		UI.LayoutPushCut(UICut.Right, 0.15f);
		UI.PanelAt(UI.LayoutAt, UI.LayoutRemaining);
		UI.PushEnabled(captured);
		UI.Label("Assign to:");
		AssignButtonLine("X1",      ref x1,      ControllerKey.X1,      Key.None,  handPose);
		AssignButtonLine("X2",      ref x2,      ControllerKey.X2,      Key.None,  handPose);
		AssignButtonLine("Stick",   ref stick,   ControllerKey.Stick,   Key.None,  handPose);
		AssignButtonLine("Menu",    ref menu,    ControllerKey.Menu,    Key.None,  handPose);
		AssignButtonLine("Grip",    ref grip,    ControllerKey.Grip,    Key.None,  handPose);
		AssignButtonLine("Trigger", ref trigger, ControllerKey.Trigger, Key.None,  handPose);
		AssignButtonLine("Space",   ref space,   ControllerKey.None,    Key.Space, handPose);
		UI.PopEnabled();
		UI.LayoutPop();

		// Pose management
		if (UI.Button("Capture Hand")) { handPose = CaptureHand(); captured = true; }
		UI.PushEnabled(captured);
		if (UI.Button("Clear Hand"  )) { captured = false; }
		UI.PopEnabled();

		UI.HSeparator();

		// UI for rotating and translating the hand pose
		if (UI.Radio("Rotate", offMode == Mode.Rotate, V.XY(0.07f, 0))) offMode = Mode.Rotate;
		UI.SameLine();
		if (UI.Radio("Translate", offMode == Mode.Translate, V.XY(0.07f, 0))) offMode = Mode.Translate;
		if (offMode == Mode.Rotate)
		{
			UI.Label("X", V.XY(0.02f, UI.LineHeight), false); UI.SameLine(); UI.HSlider("X", ref offRotX,  180, -180, 15, 0.1f, UIConfirm.Pinch);
			UI.Label("Y", V.XY(0.02f, UI.LineHeight), false); UI.SameLine(); UI.HSlider("Y", ref offRotY, -180,  180, 15, 0.1f, UIConfirm.Pinch);
			UI.Label("Z", V.XY(0.02f, UI.LineHeight), false); UI.SameLine(); UI.HSlider("Z", ref offRotZ, -180,  180, 15, 0.1f, UIConfirm.Pinch);
		}
		else
		{
			UI.Label("X", V.XY(0.02f, UI.LineHeight), false); UI.SameLine(); UI.HSlider("X", ref offPosX,  0.04f, -0.04f, 0.005f, 0.1f, UIConfirm.Pinch);
			UI.Label("Y", V.XY(0.02f, UI.LineHeight), false); UI.SameLine(); UI.HSlider("Y", ref offPosY, -0.04f,  0.04f, 0.005f, 0.1f, UIConfirm.Pinch);
			UI.Label("Z", V.XY(0.02f, UI.LineHeight), false); UI.SameLine(); UI.HSlider("Z", ref offPosZ,  0.04f, -0.04f, 0.005f, 0.1f, UIConfirm.Pinch);
		}

		// And saving results!
		UI.HSeparator();
		if (UI.Button("Save to Log")) Log.Info(HandToString(TransformPose(handPose, V.XYZ(offPosX, offPosY, offPosZ), Quat.FromAngles(offRotX, offRotY, offRotZ))));

		UI.WindowEnd();

		// Lets also show the root pose on the Controller, if it's around
		Controller c = Input.Controller(Handed.Right);
		if (c.tracked.IsActive())
			Lines.AddAxis(new Pose(c.palm.position, c.palm.orientation * Quat.FromAngles(0,90,-90)), 0.03f);

		Demo.ShowSummary(title, description,
			new Bounds(new Vec3(-.07f,-.15f,0), new Vec3(.6f, .4f, 0.2f)));
	}

	void AssignButtonLine(string name, ref HandSimId id, ControllerKey ckey, Key key, Pose[] handPose)
	{
		UI.PushId(name);
		if (UI.Button(name, V.XY(0.07f,0))) id = Input.HandSimPoseAdd(TransformPose(handPose, V.XYZ(offPosX, offPosY, offPosZ), Quat.FromAngles(offRotX, offRotY, offRotZ)), ckey, ControllerKey.None, key);
		UI.PushEnabled(id != HandSimId.None);
		UI.SameLine();
		if (UI.Button("Clear", V.XY(0.07f, 0)))
		{
			Input.HandSimPoseRemove(id);
			id = HandSimId.None;
		}
		UI.PopEnabled();
		UI.PopId();
	}

	static Pose[] CaptureHand()
	{
		Hand   hand     = Input.Hand(Handed.Right);
		Pose[] relPoses = new Pose[25];

		// The palm pose faces out the front of the hand, and we need that
		// position, but facing forward towards the fingers.
		Matrix toRelative = Matrix.TR(hand.palm.position, hand.palm.orientation).Inverse;
		for (int i = 0; i < 25; i++)
			relPoses[i] = toRelative.Transform(hand.fingers[i].Pose);

		return relPoses;
	}

	static Pose[] TransformPose(Pose[] handPose, Vec3 translate, Quat rotate)
	{
		Pose[] result = new Pose[handPose.Length];
		for (int j = 0; j < handPose.Length; j++)
		{
			result[j].position    = handPose[j].position    * rotate + translate;
			result[j].orientation = handPose[j].orientation * rotate;
		}
		return result;
	}

	static string HandToString(Pose[] handPose)
	{
		string result = "Input.HandSimPoseAdd(new Pose[]{";
		for (int j = 0; j < handPose.Length; j++)
		{
			Quat quat = handPose[j].orientation;
			Vec3 pos  = handPose[j].position;
			result += $"new Pose({pos.x:0.000}f,{pos.y:0.000}f,{pos.z:0.000}f, new Quat({quat.x:0.000}f,{quat.y:0.000}f,{quat.z:0.000}f,{quat.w:0.000}f))";
			if (j < handPose.Length - 1)
				result += ",";
		}
		result += "},\n\tControllerKey.None);";
		return result;
	}

	static void DrawHandPose(Matrix rootTransform, Vec3 offset, Vec3 rotation, Pose[] handPose, bool solid)
	{
		Hierarchy.Push(rootTransform);
		Lines.AddAxis(Pose.Identity, 0.03f);
		Hierarchy.Push(Matrix.TR(offset, rotation));
		LinePoint[] line = new LinePoint[5];
		for (int f = 0; f < 5; f++)
		{
			line[0] = new LinePoint(handPose[f*5 + 0].position, new Color32(255, 255, 255, 0), 0.006f);
			line[1] = new LinePoint(handPose[f*5 + 1].position, new Color32(255, 255, 255, (byte)(solid?160:80)), 0.006f);
			line[2] = new LinePoint(handPose[f*5 + 2].position, new Color32(255, 255, 255, (byte)(solid?255:128)), 0.004f);
			line[3] = new LinePoint(handPose[f*5 + 3].position, new Color32(255, 255, 255, (byte)(solid?255:128)), 0.004f);
			line[4] = new LinePoint(handPose[f*5 + 4].position, new Color32(255, 255, 255, (byte)(solid?255:128)), 0.002f);
			Lines.Add(line);
		}
		Hierarchy.Pop();
		Hierarchy.Pop();
	}
}