using StereoKit;

class DocOcclusion : ITest
{
	/// :CodeSample: World.OcclusionEnabled World.OcclusionMaterial SystemInfo.worldOcclusionPresent
	/// ### Basic World Occlusion
	/// 
	/// A simple example of turning on the occlusion mesh and overriding the
	/// default material so it's visible. For normal usage where you just 
	/// want to let the real world occlude geometry, the only important
	/// element is to just set `World.OcclusionEnabled = true;`.
	Material occlusionMatPrev;

	public void Start()
	{
		if (!SK.System.worldOcclusionPresent)
			Log.Info("Occlusion not available!");

		// If not available, this will have no effect
		World.OcclusionEnabled = true;

		// Override the default occluding material
		occlusionMatPrev = World.OcclusionMaterial;
		World.OcclusionMaterial = Material.Default;
	}

	public void Stop()
	{
		// Restore the previous occlusion material
		World.OcclusionMaterial = occlusionMatPrev;

		// Stop occlusion
		World.OcclusionEnabled = false;
	}
	/// :End:

	public void Initialize() => Start();
	public void Shutdown() => Stop();
	public void Update(){}
}
