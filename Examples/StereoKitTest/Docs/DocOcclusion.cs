using StereoKit;

class DocOcclusion : ITest
{
	/// :CodeSample: World.Occlusion World.OcclusionCapabilities OcclusionCaps
	/// ### Basic World Occlusion
	///
	/// A simple example of turning on occlusion. The method you use depends
	/// on what the device supports — check `World.OcclusionCapabilities` to
	/// see what's available. For example, HoloLens supports
	/// `OcclusionCaps.Mesh`, while Quest supports `OcclusionCaps.Depth`.
	OcclusionCaps prevOcclusion;

	public void Start()
	{
		OcclusionCaps available = World.OcclusionCapabilities;
		if (available == OcclusionCaps.None)
			Log.Info("Occlusion not available!");

		// Store current state so we can restore it later
		prevOcclusion = World.Occlusion;

		// Enable whatever occlusion the device supports
		World.Occlusion = available;
	}

	public void Stop()
	{
		// Restore the previous occlusion state
		World.Occlusion = prevOcclusion;
	}
	/// :End:

	public void Initialize() => Start();
	public void Shutdown() => Stop();
	public void Step(){}
}
