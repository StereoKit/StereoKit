using StereoKit;

class DocWorldSettings : ITest
{
	public void Initialize()
	{
		/// :CodeSample: World WorldRefresh World.OcclusionEnabled SystemInfo.worldOcclusionPresent World.RefreshType World.RefreshInterval World.RefreshRadius
		/// ### Configuring Quality Occlusion
		/// 
		/// If you expect the user's environment to change a lot, or you
		/// anticipate the user's environment may not be well scanned already,
		/// then you may wish to boost the frequency of world data updates. By
		/// default, StereoKit is quite conservative about scanning to reduce
		/// computation, but this can be configured using the World.RefreshX
		/// properties as seen here.
		/// 
		// If occlusion is not available, the rest of the code will have no
		// effect.
		if (!SK.System.worldOcclusionPresent)
			Log.Info("Occlusion not available!");

		// Configure SK to update the world data as fast as possible, this
		// allows occlusion to accomodate better for moving objects.
		World.OcclusionEnabled = true;
		World.RefreshType     = WorldRefresh.Timer; // Refresh on a timer
		World.RefreshInterval = 0; // Refresh every 0 seconds
		World.RefreshRadius   = 6; // Get everything in a 6m radius
		/// :End:
	}

	public void Shutdown() => World.OcclusionEnabled = false;
	public void Update() { }
}
