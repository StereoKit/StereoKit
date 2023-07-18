using StereoKit;
using System;

class DocVolumeAt : ITest
{
	public void Initialize()
	{
		Tests.Hand(new HandJoint[] { new HandJoint(new Vec3(0.026f, 0.177f, -0.363f), new Quat(-0.524f, -0.209f, -0.417f, -0.713f), 0.006f), new HandJoint(new Vec3(0.026f, 0.177f, -0.363f), new Quat(-0.271f, -0.272f, -0.553f, -0.739f), 0.015f), new HandJoint(new Vec3(-0.006f, 0.181f, -0.395f), new Quat(-0.210f, -0.221f, -0.573f, -0.761f), 0.013f), new HandJoint(new Vec3(-0.024f, 0.184f, -0.420f), new Quat(-0.092f, -0.122f, -0.599f, -0.786f), 0.011f), new HandJoint(new Vec3(-0.029f, 0.184f, -0.436f), new Quat(-0.092f, -0.122f, -0.599f, -0.786f), 0.008f), new HandJoint(new Vec3(0.038f, 0.197f, -0.360f), new Quat(-0.264f, -0.336f, 0.043f, -0.903f), 0.005f), new HandJoint(new Vec3(-0.004f, 0.233f, -0.405f), new Quat(0.114f, -0.347f, -0.100f, -0.926f), 0.013f), new HandJoint(new Vec3(-0.028f, 0.222f, -0.434f), new Quat(0.546f, -0.228f, -0.259f, -0.763f), 0.011f), new HandJoint(new Vec3(-0.029f, 0.201f, -0.440f), new Quat(0.711f, -0.147f, -0.315f, -0.612f), 0.010f), new HandJoint(new Vec3(-0.026f, 0.189f, -0.440f), new Quat(0.711f, -0.147f, -0.315f, -0.612f), 0.007f), new HandJoint(new Vec3(0.047f, 0.201f, -0.367f), new Quat(-0.235f, -0.295f, 0.139f, -0.916f), 0.005f), new HandJoint(new Vec3(0.015f, 0.236f, -0.416f), new Quat(-0.109f, -0.357f, 0.084f, -0.924f), 0.013f), new HandJoint(new Vec3(-0.013f, 0.247f, -0.447f), new Quat(0.128f, -0.358f, -0.007f, -0.925f), 0.011f), new HandJoint(new Vec3(-0.030f, 0.241f, -0.466f), new Quat(0.280f, -0.346f, -0.067f, -0.893f), 0.010f), new HandJoint(new Vec3(-0.038f, 0.234f, -0.473f), new Quat(0.280f, -0.346f, -0.067f, -0.893f), 0.007f), new HandJoint(new Vec3(0.055f, 0.199f, -0.376f), new Quat(-0.225f, -0.233f, 0.194f, -0.926f), 0.004f), new HandJoint(new Vec3(0.034f, 0.230f, -0.424f), new Quat(-0.187f, -0.291f, 0.170f, -0.923f), 0.011f), new HandJoint(new Vec3(0.015f, 0.249f, -0.455f), new Quat(-0.028f, -0.320f, 0.117f, -0.940f), 0.010f), new HandJoint(new Vec3(0.000f, 0.252f, -0.476f), new Quat(0.093f, -0.336f, 0.074f, -0.934f), 0.008f), new HandJoint(new Vec3(-0.009f, 0.250f, -0.486f), new Quat(0.093f, -0.336f, 0.074f, -0.934f), 0.006f), new HandJoint(new Vec3(0.063f, 0.192f, -0.385f), new Quat(-0.230f, -0.185f, 0.291f, -0.910f), 0.004f), new HandJoint(new Vec3(0.052f, 0.221f, -0.431f), new Quat(-0.195f, -0.220f, 0.276f, -0.915f), 0.010f), new HandJoint(new Vec3(0.042f, 0.237f, -0.458f), new Quat(-0.061f, -0.275f, 0.237f, -0.930f), 0.008f), new HandJoint(new Vec3(0.033f, 0.241f, -0.473f), new Quat(0.042f, -0.312f, 0.203f, -0.927f), 0.007f), new HandJoint(new Vec3(0.026f, 0.242f, -0.483f), new Quat(0.042f, -0.312f, 0.203f, -0.927f), 0.006f), new HandJoint(new Vec3(-0.026f, 0.189f, -0.440f), new Quat(-0.412f, 0.249f, 0.114f, 0.869f), 0.000f), new HandJoint(new Vec3(0.000f, 0.000f, 0.000f), new Quat(0.000f, 0.000f, 0.000f, 0.000f), 0.000f) });
		Tests.RunForFrames(2);
	}

	public void Shutdown()
	{
	}

	public void Step()
	{
		/// :CodeSample: UI.VolumeAt
		/// This code will draw an axis at the index finger's location when
		/// the user pinches while inside a VolumeAt.
		/// 
		/// ![UI.InteractVolume]({{site.screen_url}}/InteractVolume.jpg)
		/// 
		// Draw a transparent volume so the user can see this space
		Vec3  volumeAt   = new Vec3(0, 0.2f, -0.4f);
		float volumeSize = 0.2f;
		Default.MeshCube.Draw(Default.MaterialUIBox, Matrix.TS(volumeAt, volumeSize));

		BtnState volumeState = UI.VolumeAt("Volume", new Bounds(volumeAt, Vec3.One * volumeSize), UIConfirm.Pinch, out Handed hand);
		if (volumeState != BtnState.Inactive)
		{
			// If it just changed interaction state, make it jump in size
			float scale = volumeState.IsChanged()
				? 0.1f
				: 0.05f;
			Lines.AddAxis(Input.Hand(hand)[FingerId.Index, JointId.Tip].Pose, scale);
		}
		/// :End:

		Tests.Screenshot("InteractVolume.jpg", 1, 600, 600, 90, new Vec3(-0.102f, 0.306f, -0.240f), new Vec3(0.410f, -0.248f, -0.897f));
	}
}