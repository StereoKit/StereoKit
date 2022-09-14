using StereoKit;

class DocHand : ITest
{
	static void HandInBounds()
	{
		/// :CodeSample: Input.Hand Bounds.Contains Lines.AddAxis Hand Handed FingerId JointId
		/// Here's a small example of checking to see if a finger joint is inside
		/// a box, and drawing an axis gizmo when it is!
		// A volume for checking inside of! 10cm on each side, at the origin
		Bounds testArea = new Bounds(Vec3.One * 0.1f);

		// This is a decent way to show we're working with both hands
		for (int h = 0; h < (int)Handed.Max; h++)
		{
			// Get the pose for the index fingertip
			Hand hand      = Input.Hand((Handed)h);
			Pose fingertip = hand[FingerId.Index, JointId.Tip].Pose;

			// Skip this hand if it's not tracked
			if (!hand.IsTracked) continue;

			// Draw the fingertip pose axis if it's inside the volume
			if (testArea.Contains(fingertip.position))
				Lines.AddAxis(fingertip);
		}
		/// :End:
	}

	public void Initialize() { }
	public void Shutdown() { }
	public void Update() {
		HandInBounds();

		Hand hand = Input.Hand(Handed.Right);
		hand.Visible = hand.IsGripped;
	}
}