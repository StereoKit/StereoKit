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
	float[]  micBuffer = new float[400];
	float    micIntensity     = 0;
	float    micIntensityDest = 0;

	public void Initialize()
	{
		micSprite   = Sprite.FromFile("mic_icon.png", SpriteType.Single);
		micMaterial = Default.MaterialUnlit.Copy();
		micMaterial.Transparency = Transparency.Blend;

		string[] micDevices = Microphone.GetDevices();
		Microphone.Start();

		Tests.RunForFrames(2);
	}

	public void Shutdown()
	{
		Microphone.Stop();
	}

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
		Tests.Screenshot(1, 400, 400, "MicrophoneSelector.jpg", micSelectPose.position+V.XYZ(-0.15f, -0.02f, 0.15f), micSelectPose.position-V.XYZ(0,0.12f,0));

		if (Microphone.IsRecording)
		{
			if (Microphone.Sound.UnreadSamples > micBuffer.Length)
				micBuffer = new float[Microphone.Sound.UnreadSamples];
			int samples = Microphone.Sound.ReadSamples(ref micBuffer);
			if (samples > 0)
			{
				float avg = 0;
				for (int i = 0; i < samples; i++)
					avg += Math.Abs(micBuffer[i]);
				avg = avg / samples;
				avg = Math.Max(0, 1 - avg);
				micIntensityDest = 1 - (avg * avg);
			}

			micIntensity = SKMath.Lerp(micIntensity, micIntensityDest, 16 * Time.Elapsedf);
			float scale = 0.1f + 0.1f * micIntensity;
			Color color = new Color(1,1,1, Math.Max(0.1f, micIntensity));
			Default.MeshSphere.Draw(micMaterial, Matrix.TS(0,0,-0.5f, scale), color);
			micSprite.Draw(Matrix.TS(-0.03f,0.03f,-0.5f, 0.06f));
		}
		else
		{
			Default.MeshSphere.Draw(micMaterial, Matrix.TS(0, 0, -0.5f, 0.1f), new Color(1,0,0,0.1f));
			micSprite.Draw(Matrix.TS(-0.03f, 0.03f, -0.5f, 0.06f));
		}

		Text.Add(title,       titlePose);
		Text.Add(description, descPose, V.XY(0.4f, 0), TextFit.Wrap, TextAlign.XCenter | TextAlign.YTop, TextAlign.XLeft | TextAlign.YTop);
	}
}