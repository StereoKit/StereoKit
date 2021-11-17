using StereoKit;

class DocWorldRaycast : ITest
{
	/// :CodeSample: World.RaycastEnabled World.Raycast SystemInfo.worldRaycastPresent
	/// ### Basic World Raycasting
	/// 
	/// World.RaycastEnabled must be true before calling World.Raycast, or
	/// you won't ever intersect with any world geometry.
	public void Start()
	{
		if (!SK.System.worldRaycastPresent)
			Log.Info("World raycasting not available!");

		// This must be enabled before calling World.Raycast
		World.RaycastEnabled = true;
	}

	public void Stop() => World.RaycastEnabled = false;

	public void Step()
	{
		// Raycast out the index finger of each hand, and draw a red sphere
		// at the intersection point.
		for (int i = 0; i < 2; i++)
		{
			Hand hand = Input.Hand(i);
			if (!hand.IsTracked) continue;

			Ray fingerRay = hand[FingerId.Index, JointId.Tip].Pose.Ray;
			if (World.Raycast(fingerRay, out Ray at))
				Mesh.Sphere.Draw(Material.Default, Matrix.TS(at.position, 0.03f), new Color(1, 0, 0));
		}
	}
	/// :End:

	public void Initialize() => Start();
	public void Shutdown() => Stop();
	public void Update() => Step();
}