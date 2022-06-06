/// :CodeDoc: Guides 3 Using Hands
/// # Using Hands
/// 
/// StereoKit uses a hands first approach to user input! Even when hand-sensors
/// aren't available, hand data is simulated instead using existing devices!
/// For example, Windows Mixed Reality controllers will blend between pre-recorded
/// hand poses based on button presses, as will mice. This way, fully articulated 
/// hand data is always present for you to work with!
/// 
/// :End:

using StereoKit;
using StereoKit.Framework;

class DemoHands : ITest
{
	Matrix descPose    = Matrix.TR (-0.5f, 0, -0.5f, Quat.LookDir(1,0,1));
	string description = "StereoKit uses a hands first approach to user input! Even when hand-sensors aren't available, hand data is simulated instead using existing devices. Check out Input.Hand for all the cool data you get!\n\nThis demo is the source for the 'Using Hands' guide, and is a collection of different options and examples of how to get, use, and visualize Hand data.";
	Matrix titlePose   = Matrix.TRS(V.XYZ(-0.5f, 0.05f, -0.5f), Quat.LookDir(1, 0, 1), 2);
	string title       = "Hand Input";

	static Pose optionsPose = new Pose(0.5f,0,-0.5f, Quat.LookDir(-1,0,1));
	bool showHands     = true;
	bool showJoints    = false;
	bool showAxes      = true;
	bool showPointers  = true;
	bool showHandMenus = true;
	bool showHandSize  = true;

	Mesh jointMesh = Mesh.GenerateSphere(1);
	HandMenuRadial handMenu;

	public void Initialize() {
		/// :CodeDoc: Guides Using Hands
		/// ## Accessing Joints
		/// 
		/// ![Hand with joints]({{site.url}}/img/screenshots/HandAxes.jpg)
		/// 
		/// Since hands are so central to interaction, accessing hand information needs
		/// to be really easy to get! So here's how you might find the fingertip of the right
		/// hand! If you ignore IsTracked, this'll give you the last known position for that
		/// finger joint.
		Hand hand = Input.Hand(Handed.Right);
		if (hand.IsTracked)
		{ 
			Vec3 fingertip = hand[FingerId.Index, JointId.Tip].position;
		}
		/// Pretty straightforward! And if you prefer calling a function instead of using the
		/// [] operator, that's cool too! You can call `hand.Get(FingerId.Index, JointId.Tip)`
		/// instead!
		/// 
		/// If that's too granular for you, there's easy ways to check for pinching and 
		/// gripping! Pinched will tell you if a pinch is currently happening, JustPinched
		/// will tell you if it just started being pinched this frame, and JustUnpinched will
		/// tell you if the pinch just stopped this frame!
		if (hand.IsPinched) { }
		if (hand.IsJustPinched) { }
		if (hand.IsJustUnpinched) { }

		if (hand.IsGripped) { }
		if (hand.IsJustGripped) { }
		if (hand.IsJustUngripped) { }
		/// These are all convenience functions wrapping the `hand.pinchState` bit-flag, so you
		/// can also use that directly if you want to do some bit-flag wizardry!
		/// :End:

		/// :CodeSample: HandMenuRadial HandRadialLayer HandMenuItem
		/// ### Basic layered hand menu
		/// 
		/// The HandMenuRadial is an `IStepper`, so it should be registered with 
		/// `StereoKitApp.AddStepper` so it can run by itself! It's recommended to
		/// keep track of it anyway, so you can remove it when you're done with it
		/// via `StereoKitApp.RemoveStepper`
		/// 
		/// The constructor uses a params style argument list that makes it easy and
		/// clean to provide lists of items! This means you can assemble the whole
		/// menu on a single 'line'. You can still pass arrays instead if you prefer
		/// that!
		handMenu = SK.AddStepper(new HandMenuRadial(
			new HandRadialLayer("Root",
				new HandMenuItem("File",   null, null, "File"),
				new HandMenuItem("Edit",   null, null, "Edit"),
				new HandMenuItem("About",  null, () => Log.Info(SK.VersionName)),
				new HandMenuItem("Cancel", null, null)),
			new HandRadialLayer("File", 
				new HandMenuItem("New",   null, () => Log.Info("New")),
				new HandMenuItem("Open",  null, () => Log.Info("Open")),
				new HandMenuItem("Close", null, () => Log.Info("Close")),
				new HandMenuItem("Back",  null, null, HandMenuAction.Back)),
			new HandRadialLayer("Edit",
				new HandMenuItem("Copy",  null, () => Log.Info("Copy")),
				new HandMenuItem("Paste", null, () => Log.Info("Paste")),
				new HandMenuItem("Back", null, null, HandMenuAction.Back))));
		/// :End:

		Tests.RunForFrames(2);
		Tests.Hand(new HandJoint[] { new HandJoint(new Vec3(-0.529f, -0.198f, -0.126f), new Quat(-0.744f, -0.530f, 0.156f, -0.376f), 0.004f), new HandJoint(new Vec3(-0.529f, -0.198f, -0.126f), new Quat(-0.744f, -0.530f, 0.156f, -0.376f), 0.010f), new HandJoint(new Vec3(-0.533f, -0.175f, -0.090f), new Quat(-0.786f, -0.550f, 0.126f, -0.254f), 0.009f), new HandJoint(new Vec3(-0.544f, -0.158f, -0.069f), new Quat(-0.729f, -0.564f, 0.027f, -0.387f), 0.008f), new HandJoint(new Vec3(-0.557f, -0.150f, -0.065f), new Quat(-0.585f, -0.548f, -0.140f, -0.582f), 0.006f), new HandJoint(new Vec3(-0.521f, -0.182f, -0.136f), new Quat(-0.277f, -0.826f, 0.317f, -0.376f), 0.004f), new HandJoint(new Vec3(-0.550f, -0.135f, -0.102f), new Quat(-0.277f, -0.826f, 0.317f, -0.376f), 0.009f), new HandJoint(new Vec3(-0.571f, -0.112f, -0.082f), new Quat(-0.244f, -0.843f, 0.256f, -0.404f), 0.008f), new HandJoint(new Vec3(-0.585f, -0.102f, -0.070f), new Quat(-0.200f, -0.866f, 0.165f, -0.428f), 0.007f), new HandJoint(new Vec3(-0.593f, -0.098f, -0.064f), new Quat(-0.172f, -0.874f, 0.110f, -0.440f), 0.005f), new HandJoint(new Vec3(-0.527f, -0.178f, -0.144f), new Quat(-0.185f, -0.817f, 0.370f, -0.401f), 0.004f), new HandJoint(new Vec3(-0.559f, -0.132f, -0.119f), new Quat(-0.185f, -0.817f, 0.370f, -0.401f), 0.009f), new HandJoint(new Vec3(-0.582f, -0.101f, -0.104f), new Quat(-0.175f, -0.809f, 0.371f, -0.420f), 0.008f), new HandJoint(new Vec3(-0.599f, -0.089f, -0.092f), new Quat(-0.109f, -0.856f, 0.245f, -0.443f), 0.007f), new HandJoint(new Vec3(-0.608f, -0.084f, -0.086f), new Quat(-0.075f, -0.871f, 0.180f, -0.450f), 0.005f), new HandJoint(new Vec3(-0.535f, -0.178f, -0.152f), new Quat(-0.132f, -0.786f, 0.408f, -0.445f), 0.003f), new HandJoint(new Vec3(-0.568f, -0.136f, -0.137f), new Quat(-0.132f, -0.786f, 0.408f, -0.445f), 0.008f), new HandJoint(new Vec3(-0.590f, -0.106f, -0.130f), new Quat(-0.131f, -0.762f, 0.432f, -0.464f), 0.007f), new HandJoint(new Vec3(-0.607f, -0.092f, -0.122f), new Quat(-0.071f, -0.810f, 0.332f, -0.477f), 0.006f), new HandJoint(new Vec3(-0.617f, -0.086f, -0.117f), new Quat(-0.029f, -0.836f, 0.260f, -0.482f), 0.004f), new HandJoint(new Vec3(-0.544f, -0.183f, -0.159f), new Quat(-0.060f, -0.749f, 0.481f, -0.452f), 0.003f), new HandJoint(new Vec3(-0.576f, -0.143f, -0.152f), new Quat(-0.060f, -0.749f, 0.481f, -0.452f), 0.007f), new HandJoint(new Vec3(-0.594f, -0.119f, -0.154f), new Quat(-0.061f, -0.684f, 0.534f, -0.493f), 0.006f), new HandJoint(new Vec3(-0.607f, -0.108f, -0.152f), new Quat(0.002f, -0.745f, 0.444f, -0.498f), 0.005f), new HandJoint(new Vec3(-0.616f, -0.102f, -0.150f), new Quat(0.045f, -0.780f, 0.378f, -0.496f), 0.004f), new HandJoint(new Vec3(-0.548f, -0.161f, -0.137f), new Quat(-0.267f, 0.849f, 0.204f, 0.407f), 0.000f), new HandJoint(new Vec3(-0.548f, -0.161f, -0.137f), new Quat(-0.267f, 0.849f, 0.204f, 0.407f), 0.000f) });
	}

	public void Shutdown()
	{
		/// :CodeSample: HandMenuRadial HandRadialLayer HandMenuItem
		SK.RemoveStepper(handMenu); 
		/// :End:
	}

	public void Update()
	{
		UI.WindowBegin("Options", ref optionsPose, new Vec2(24, 0)*U.cm);
		UI.Label("Show");
		if (UI.Toggle("Hands", ref showHands))
			Input.HandVisible(Handed.Max, showHands);
		UI.SameLine();
		UI.Toggle("Joints", ref showJoints);
		UI.SameLine();
		UI.Toggle("Axes", ref showAxes);
		UI.SameLine();
		UI.Toggle("Hand Size", ref showHandSize);
		UI.SameLine();
		UI.Toggle("Pointers", ref showPointers);
		UI.SameLine();
		UI.Toggle("Menu", ref showHandMenus);
		UI.Label("Color");
		if (UI.Button("Rainbow"))
			ColorizeFingers(16, 
				new Gradient(
					new GradientKey(Color.HSV(0.0f,1,1), 0.1f),
					new GradientKey(Color.HSV(0.2f,1,1), 0.3f),
					new GradientKey(Color.HSV(0.4f,1,1), 0.5f),
					new GradientKey(Color.HSV(0.6f,1,1), 0.7f),
					new GradientKey(Color.HSV(0.8f,1,1), 0.9f)),
				new Gradient(
					new GradientKey(new Color(1,1,1,0), 0),
					new GradientKey(new Color(1,1,1,0), 0.4f),
					new GradientKey(new Color(1,1,1,1), 0.9f)));
		UI.SameLine();
		if (UI.Button("Black"))
			ColorizeFingers(16,
				new Gradient(new GradientKey(new Color(0,0,0,1), 1)),
				new Gradient(
					new GradientKey(new Color(1,1,1,0), 0),
					new GradientKey(new Color(1,1,1,0), 0.4f),
					new GradientKey(new Color(1,1,1,1), 0.6f),
					new GradientKey(new Color(1,1,1,1), 0.9f)));
		UI.SameLine();
		if (UI.Button("Full Black"))
			ColorizeFingers(16,
				new Gradient(new GradientKey(new Color(0, 0, 0, 1), 1)),
				new Gradient(
					new GradientKey(new Color(1, 1, 1, 0), 0),
					new GradientKey(new Color(1, 1, 1, 1), 0.05f),
					new GradientKey(new Color(1, 1, 1, 1), 1.0f)));
		UI.SameLine();
		if (UI.Button("Normal"))
			ColorizeFingers(16,
				new Gradient(new GradientKey(new Color(1, 1, 1, 1), 1)),
				new Gradient(
					new GradientKey(new Color(.4f,.4f,.4f,0), 0),
					new GradientKey(new Color(.6f,.6f,.6f,0), 0.4f),
					new GradientKey(new Color(.8f,.8f,.8f,1), 0.55f),
					new GradientKey(new Color(1,1,1,1),       1)));
		UI.WindowEnd();

		if (showJoints)   DrawJoints(jointMesh, Default.Material);
		if (showAxes)     DrawAxes();
		if (showPointers) DrawPointers();
		if (showHandSize) DrawHandSize();
		if (showHandMenus) 
		{ 
			DrawHandMenu(Handed.Right);
			DrawHandMenu(Handed.Left);
		}

		Tests.Screenshot("HandAxes.jpg", 1, 600, 600, 90, new Vec3(-0.508f, -0.082f, -0.061f), new Vec3(-1.219f, -0.651f, -0.474f));

		if (!Tests.IsTesting) { 
			Text.Add(title, titlePose);
			Text.Add(description, descPose, V.XY(0.4f, 0), TextFit.Wrap, TextAlign.TopCenter, TextAlign.TopLeft);
		}
	}

	private void ColorizeFingers(int size, Gradient horizontal, Gradient vertical)
	{
		Tex tex = new Tex(TexType.Image, TexFormat.Rgba32Linear);
		tex.AddressMode = TexAddress.Clamp;

		Color32[] pixels = new Color32[size*size];
		for (int y = 0; y < size; y++)
		{
			Color v = vertical.Get(1 - y / (size-1.0f));
			for (int x = 0; x < size; x++)
			{
				Color h = horizontal.Get(x / (size-1.0f));
				pixels[x+y*size] = v*h;
			}
		}
		tex.SetColors(size, size, pixels);

		Default.MaterialHand[MatParamName.DiffuseTex] = tex;
	}

	/// :CodeDoc: Guides Using Hands
	/// ## Hand Menu
	/// 
	/// Lets imagine you want to make a hand menu, you might need to know 
	/// if the user is looking at the palm of their hand! Here's a quick 
	/// example of using the palm's pose and the dot product to determine 
	/// this.
	static bool HandFacingHead(Handed handed)
	{
		Hand hand = Input.Hand(handed);
		if (!hand.IsTracked)
			return false;

		Vec3 palmDirection   = (hand.palm.Forward).Normalized;
		Vec3 directionToHead = (Input.Head.position - hand.palm.position).Normalized;

		return Vec3.Dot(palmDirection, directionToHead) > 0.5f;
	}
	/// Once you have that information, it's simply a matter of placing a
	/// window off to the side of the hand! The palm pose Right direction
	/// points to different sides of each hand, so a different X offset
	/// is required for each hand.
	public static void DrawHandMenu(Handed handed)
	{
		if (!HandFacingHead(handed))
			return;

		// Decide the size and offset of the menu
		Vec2  size   = new Vec2(4, 16);
		float offset = handed == Handed.Left ? -2-size.x : 2+size.x;

		// Position the menu relative to the side of the hand
		Hand hand   = Input.Hand(handed);
		Vec3 at     = hand[FingerId.Little, JointId.KnuckleMajor].position;
		Vec3 down   = hand[FingerId.Little, JointId.Root        ].position;
		Vec3 across = hand[FingerId.Index,  JointId.KnuckleMajor].position;

		Pose menuPose = new Pose(
			at,
			Quat.LookAt(at, across, at-down) * Quat.FromAngles(0, handed == Handed.Left ? 90 : -90, 0));
		menuPose.position += menuPose.Right * offset * U.cm;
		menuPose.position += menuPose.Up * (size.y/2) * U.cm;

		// And make a menu!
		UI.WindowBegin("HandMenu", ref menuPose, size * U.cm, UIWin.Empty);
		UI.Button("Test");
		UI.Button("That");
		UI.Button("Hand");
		UI.WindowEnd();
	}
	/// :End:

	public static void DrawAxes()
	{
		for (int i = 0; i < (int)Handed.Max; i++) {
			Hand hand = Input.Hand((Handed)i);
			if (!hand.IsTracked)
				continue;

			for (int finger = 0; finger < 5; finger++) {
			for (int joint  = 0; joint  < 5; joint++ ) {
				Lines.AddAxis(hand[finger, joint].Pose);
			}}
			Lines.AddAxis(hand.palm);
		}
	}

	public static void DrawJoints(Mesh jointMesh, Material jointMaterial)
	{
		for (int i = 0; i < (int)Handed.Max; i++) {
			Hand hand = Input.Hand((Handed)i);
			if (!hand.IsTracked)
				continue;

			for (int finger = 0; finger < 5; finger++) {
			for (int joint  = 0; joint  < 5; joint++ ) {
				HandJoint current = hand[finger, joint];
				jointMesh.Draw(jointMaterial, Matrix.TRS(current.position, current.orientation, current.radius/2));
			} }
		}
	}

	/// :CodeDoc: Guides Using Hands
	/// ## Pointers
	/// 
	/// And lastly, StereoKit also has a pointer system! This applies to
	/// more than just hands. Head, mouse, and other devices will also
	/// create pointers into the scene. You can filter pointers based on
	/// source family and device capabilities, so this is a great way to 
	/// abstract a few more input sources nicely!
	public static void DrawPointers()
	{
		int hands = Input.PointerCount(InputSource.Hand);
		for (int i = 0; i < hands; i++)
		{
			Pointer pointer = Input.Pointer(i, InputSource.Hand);
			Lines.Add    (pointer.ray, 0.5f, Color.White, Units.mm2m);
			Lines.AddAxis(pointer.Pose);
		}
	}
	/// :End:

	public static void DrawHandSize()
	{
		for (int h = 0; h < (int)Handed.Max; h++)
		{
			Hand hand = Input.Hand((Handed)h);
			if (!hand.IsTracked)
				continue;

			HandJoint at = hand[FingerId.Middle, JointId.Tip];
			Vec3 pos = at.position + at.Pose.Forward * at.radius;
			Quat rot = at.orientation * Quat.FromAngles(-90, 0, 0);
			if (!HandFacingHead((Handed)h)) rot = rot * Quat.FromAngles(0,180,0);

			Text.Add(
				(hand.size * 100).ToString(".0")+"cm", 
				Matrix.TRS(pos, rot, 0.3f),
				TextAlign.XCenter|TextAlign.YBottom);
		}
	}
}

/// :CodeDoc: Guides Using Hands
/// The code in context for this document can be found [on Github here](https://github.com/StereoKit/StereoKit/blob/master/Examples/StereoKitTest/DemoHands.cs)!
/// :End: