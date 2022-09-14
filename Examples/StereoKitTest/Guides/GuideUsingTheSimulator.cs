using StereoKit;

/// :CodeDoc: Guides 3 Using The Simulator
/// # Using the Simulator
/// 
/// As a developer, you can't realistically spend all of your development in
/// a headset just yet. So, a decent grasp over StereoKit's fallback
/// flatscreen MR simulator is particularly helpful! This is basically a 2D
/// window that allows you to move around and interact, without requiring an
/// OpenXR runtime or headset.
/// 
/// ## Simulator Controls
/// 
/// When you start the simulator, you'll find that your mouse controls the
/// right hand by default. This is a complete simulation of an articulated
/// hand, so you'll have access to all the joints the same way you would a
/// real tracked hand. The hand becomes tracked when the mouse enters the
/// window, and untracked when leaving the window. The pointer ray, which is
/// normally a shoulder->hand ray, will be along the mouse ray instead.
/// 
/// ### Mouse Controls:
/// - Left Mouse - Hand animates to a Pinch gesture.
/// - Right Mouse - Hand animates to a Grip gesture.
/// - Left + Right - Hand animates to a closed fist.
/// - Scroll Wheel - Moves the hand toward or away from the user.
/// - Shift + Right - Mouse-look / rotate the head.
/// - Left Alt - [Eye tracking](({{site.url}}/Pages/Reference/Input/Eyes.html) will point along the ray indicated by the mouse.
/// 
/// To move around in space, you'll find controls that should be familiar to
/// those that play first-person games! Hold Left Shift to enable this.
/// 
/// ### Keyboard Controls:
/// - Shift+W - Move forward.
/// - Shift+A - Move left.
/// - Shift+S - Move backwards.
/// - Shift+D - Move right.
/// - Shift+Q - Move down.
/// - Shift+E - Move up.
/// 
/// ## Simulator API
/// 
/// There's a few bits of functionality that let you set up the simulator, or
/// some features that may assist you in debugging or testing! Here's a
/// couple you may want to know about:
/// 
/// ### Simulator Enable/Disable
/// 
/// By default, StereoKit will fall back to the flatscreen simulator if
/// OpenXR fails to initialize for any reason (like, headset not plugged in,
/// or OpenXR not present). You can modify this behavior at initialization
/// time when defining your SKSettings for SK.Init.
/// :End:

class GuideUsingTheSimulator : ITest
{
	public void Initialize()
	{
		/// :CodeDoc: Guides Using The Simulator
		SKSettings settings = new SKSettings {
			appName                = "Flatscreen Simulator",
			assetsFolder           = "Assets",
			// This tells StereoKit to always start in a 2D flatscreen
			// window, instead of an immersive MR environment.
			displayPreference      = DisplayMode.Flatscreen,
			// Setting this to true will disable all built-in MR simulator
			// controls.
			disableFlatscreenMRSim = false,
			// Setting this to true will prevent StereoKit from creating the
			// fallback simulator when OpenXR fails to initialize. This is
			// important when shipping a final application to users.
			noFlatscreenFallback   = true,
		};
		///
		/// ### Overriding Hands
		/// 
		/// A number of functions are present that can make unit test and
		/// complex input simulation possible. For a full example of this,
		/// the [DebugToolWindow](https://github.com/StereoKit/StereoKit/blob/master/Examples/StereoKitTest/DebugToolWindow.cs)
		/// in the Test project has a number of sample utilities for
		/// recording and playing back input.
		/// 
		/// Overriding the hands is one important element that you may want
		/// to do! [`Input.HandOverride`]({{site.url}}/Pages/Reference/Input/HandOverride.html)
		/// will set the hand input to a very specific pose, and hold that
		/// pose until you call `Input.HandOverride` again with a new pose,
		/// or call [`Input.HandClearOverride`]({{site.url}}/Pages/Reference/Input/HandClearOverride.html)
		/// to restore control back to the user.
		/// 
		/// ![An overridden hand]({{site.screen_url}}/HandOverride.jpg)
		/// _This screenshot is generated fresh every StereoKit release using Input.HandOverride, to ensure consistency!_
		// These 25 joints were printed using code from a session with a real
		// hand.
		HandJoint[] joints = new HandJoint[] { new HandJoint(new Vec3(0.132f, -0.221f, -0.168f), new Quat(-0.445f, -0.392f, 0.653f, -0.472f), 0.021f), new HandJoint(new Vec3(0.132f, -0.221f, -0.168f), new Quat(-0.445f, -0.392f, 0.653f, -0.472f), 0.021f), new HandJoint(new Vec3(0.141f, -0.181f, -0.181f), new Quat(-0.342f, -0.449f, 0.618f, -0.548f), 0.014f), new HandJoint(new Vec3(0.139f, -0.151f, -0.193f), new Quat(-0.409f, -0.437f, 0.626f, -0.499f), 0.010f), new HandJoint(new Vec3(0.141f, -0.133f, -0.198f), new Quat(-0.409f, -0.437f, 0.626f, -0.499f), 0.010f), new HandJoint(new Vec3(0.124f, -0.229f, -0.172f), new Quat(0.135f, -0.428f, 0.885f, -0.125f), 0.024f), new HandJoint(new Vec3(0.103f, -0.184f, -0.209f), new Quat(0.176f, -0.530f, 0.774f, -0.299f), 0.013f), new HandJoint(new Vec3(0.078f, -0.153f, -0.225f), new Quat(0.173f, -0.645f, 0.658f, -0.349f), 0.010f), new HandJoint(new Vec3(0.061f, -0.135f, -0.228f), new Quat(-0.277f, 0.674f, -0.623f, 0.283f), 0.010f), new HandJoint(new Vec3(0.050f, -0.125f, -0.227f), new Quat(-0.277f, 0.674f, -0.623f, 0.283f), 0.010f), new HandJoint(new Vec3(0.119f, -0.235f, -0.172f), new Quat(0.147f, -0.399f, 0.847f, -0.318f), 0.024f), new HandJoint(new Vec3(0.088f, -0.200f, -0.211f), new Quat(0.282f, -0.603f, 0.697f, -0.268f), 0.012f), new HandJoint(new Vec3(0.056f, -0.169f, -0.216f), new Quat(-0.370f, 0.871f, -0.308f, 0.099f), 0.010f), new HandJoint(new Vec3(0.045f, -0.156f, -0.195f), new Quat(-0.463f, 0.884f, -0.022f, -0.066f), 0.010f), new HandJoint(new Vec3(0.047f, -0.155f, -0.178f), new Quat(-0.463f, 0.884f, -0.022f, -0.066f), 0.010f), new HandJoint(new Vec3(0.111f, -0.244f, -0.173f), new Quat(0.182f, -0.436f, 0.778f, -0.414f), 0.022f), new HandJoint(new Vec3(0.074f, -0.213f, -0.205f), new Quat(-0.353f, 0.622f, -0.656f, 0.244f), 0.011f), new HandJoint(new Vec3(0.046f, -0.189f, -0.204f), new Quat(-0.436f, 0.891f, -0.073f, -0.108f), 0.010f), new HandJoint(new Vec3(0.048f, -0.184f, -0.182f), new Quat(-0.451f, 0.811f, 0.264f, -0.263f), 0.010f), new HandJoint(new Vec3(0.061f, -0.188f, -0.168f), new Quat(-0.451f, 0.811f, 0.264f, -0.263f), 0.010f), new HandJoint(new Vec3(0.105f, -0.250f, -0.170f), new Quat(0.219f, -0.470f, 0.678f, -0.521f), 0.020f), new HandJoint(new Vec3(0.062f, -0.228f, -0.196f), new Quat(-0.444f, 0.610f, -0.623f, 0.206f), 0.010f), new HandJoint(new Vec3(0.044f, -0.215f, -0.192f), new Quat(-0.501f, 0.841f, -0.094f, -0.183f), 0.010f), new HandJoint(new Vec3(0.048f, -0.209f, -0.176f), new Quat(-0.521f, 0.682f, 0.251f, -0.448f), 0.010f), new HandJoint(new Vec3(0.061f, -0.207f, -0.168f), new Quat(-0.521f, 0.682f, 0.251f, -0.448f), 0.010f), new HandJoint(new Vec3(0.098f, -0.222f, -0.191f), new Quat(0.308f, -0.906f, 0.288f, -0.042f), 0.000f), new HandJoint(new Vec3(0.131f, -0.251f, -0.164f), new Quat(0.188f, -0.436f, 0.844f, -0.248f), 0.000f) };
		
		Input.HandOverride(Handed.Right, joints);
		/// 
		/// :End:

		if (Tests.IsTesting)
			Input.HandVisible(Handed.Right, true);
		Tests.RunForFrames(2);
	}

	public void Shutdown()
	{
		Input.HandClearOverride(Handed.Right);
	}

	public void Update()
	{
		Tests.Screenshot("HandOverride.jpg", 1, 600, 600, 45, new Vec3(0.055f, -0.047f, 0.047f), new Vec3(0.155f, -0.500f, -0.838f));
	}
}