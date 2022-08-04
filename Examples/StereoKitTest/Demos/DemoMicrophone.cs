using StereoKit;
using System;

class DemoMicrophone : ITest
{
	Matrix descPose    = Matrix.TR (-0.5f, 0, -0.5f, Quat.LookDir(1,0,1));
	string description = "Sometimes you need direct access to the microphone data! Maybe for a special effect, or maybe you just need to stream it to someone else. Well, there's an easy API for that!\n\nThis demo shows how to grab input from the microphone, and use it to drive an indicator that tells users that you're listening!";
	Matrix titlePose   = Matrix.TRS(V.XYZ(-0.5f, 0.05f, -0.5f), Quat.LookDir(1, 0, 1), 2);
	string title       = "Microphone";

	Sprite   micSprite;
	Material micMaterial;

	public void Initialize()
	{
		micSprite   = Sprite.FromFile("mic_icon.png", SpriteType.Single);
		micMaterial = Default.MaterialUnlit.Copy();
		micMaterial.Transparency = Transparency.Blend;

		Microphone.Start();

		Tests.RunForFrames(2);
	}

	public void Shutdown()
	{
		Microphone.Stop();
	}

	/// :CodeSample: Microphone Sound Microphone.Sound Microphone.IsRecording Sound.UnreadSamples Sound.ReadSamples
	/// ### Getting streaming sound intensity
	/// This example shows how to read data from a Sound stream such as the
	/// microphone! In this case, we're just finding the average 'intensity'
	/// of the audio, and returning it as a value approximately between 0 and 1.
	/// Microphone.Start() should be called before this example :)
	float[] micBuffer    = new float[0];
	float   micIntensity = 0;
	float GetMicIntensity()
	{
		if (!Microphone.IsRecording) return 0;

		// Ensure our buffer of samples is large enough to contain all the
		// data the mic has ready for us this frame
		if (Microphone.Sound.UnreadSamples > micBuffer.Length)
			micBuffer = new float[Microphone.Sound.UnreadSamples];

		// Read data from the microphone stream into our buffer, and track 
		// how much was actually read. Since the mic data collection runs in
		// a separate thread, this will often be a little inconsistent. Some
		// frames will have nothing ready, and others may have a lot!
		int samples = Microphone.Sound.ReadSamples(ref micBuffer);

		// This is a cumulative moving average over the last 1000 samples! We
		// Abs() the samples since audio waveforms are half negative.
		for (int i = 0; i < samples; i++)
			micIntensity = (micIntensity*999.0f + Math.Abs(micBuffer[i]))/1000.0f;

		return micIntensity;
	}
	/// :End:

	/// :CodeSample: Microphone Microphone.GetDevices Microphone.Start
	/// ### Choosing a microphone device
	/// While generally you'll prefer to use the default device, it can be
	/// nice to allow users to pick which mic they're using! This is
	/// especially important on PC, where users may have complicated or
	/// interesting setups.
	/// 
	/// ![Microphone device selection window]({{site.screen_url}}/MicrophoneSelector.jpg)
	/// 
	/// This sample is a very simple window that allows users to start
	/// recording with a device other than the default. NOTE: this example
	/// is designed with the assumption that Microphone.Start() has been
	/// called already.
	Pose     micSelectPose   = new Pose(0.5f, 0, -0.5f, Quat.LookDir(-1, 0, 1));
	string[] micDevices      = null;
	string   micDeviceActive = null;
	void ShowMicDeviceWindow()
	{
		// Let the user choose a microphone device
		UI.WindowBegin("Available Microphones:", ref micSelectPose);

		// User may plug or unplug a mic device, so it's nice to be able to
		// refresh this list.
		if (UI.Button("Refresh") || micDevices == null)
			micDevices = Microphone.GetDevices();
		UI.HSeparator();

		// Display the list of potential microphones. Some systems may only
		// have the default (null) device available.
		Vec2 size = V.XY(0.25f, UI.LineHeight);
		if (UI.Radio("Default", micDeviceActive == null, size))
		{
			micDeviceActive = null;
			Microphone.Start(micDeviceActive);
		}
		foreach (string device in micDevices)
		{
			if (UI.Radio(device, micDeviceActive == device, size))
			{
				micDeviceActive = device;
				Microphone.Start(micDeviceActive);
			}
		}

		UI.WindowEnd();
	}
	/// :End:

	public void Update()
	{
		ShowMicDeviceWindow();
		Tests.Screenshot("MicrophoneSelector.jpg", 1, 400, 400, 90, micSelectPose.position+V.XYZ(-0.15f, -0.02f, 0.15f), micSelectPose.position-V.XYZ(0,0.12f,0));

		if (Microphone.IsRecording)
		{
			// Squaring a 0-1 value gives an extra slow initial response, but
			// squaring the inverse of a 0-1 value and then inverting it back
			// gives a fast initial response! Try graphing "y = 1-(1-x)^2"
			// versus "y = x^2" for the x 0-1 range.
			float intensity = GetMicIntensity();
			intensity = 1 - intensity;
			intensity = 1 - (intensity * intensity);

			float scale = 0.1f + 0.06f * intensity;
			Color color = new Color(1,1,1, Math.Max(0.1f, intensity));
			Default.MeshSphere.Draw(micMaterial, Matrix.TS(0,0,-0.5f, scale), color);
			micSprite.Draw(Matrix.TS(0,0,-0.5f, 0.06f), TextAlign.Center);
		}
		else
		{
			// Draw it in red if we're not recording
			Default.MeshSphere.Draw(micMaterial, Matrix.TS(0, 0, -0.5f, 0.1f), new Color(1,0,0,0.1f));
			micSprite.Draw(Matrix.TS(0,0,-0.5f,0.06f), TextAlign.Center);
		}

		Text.Add(title,       titlePose);
		Text.Add(description, descPose, V.XY(0.4f, 0), TextFit.Wrap, TextAlign.TopCenter, TextAlign.TopLeft);
	}
}