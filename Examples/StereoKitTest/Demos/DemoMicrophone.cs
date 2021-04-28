using StereoKit;
using System;

class DemoMicrophone : ITest
{
	Matrix descPose    = Matrix.TR (-0.5f, 0, -0.5f, Quat.LookDir(1,0,1));
	string description = "Sometimes you need direct access to the microphone data! Maybe for a special effect, or maybe you just need to stream it to someone else. Well, there's an easy API for that!\n\nThis demo shows how to grab input from the microphone, and use it to drive an indicator that tells users that you're listening!";
	Matrix titlePose   = Matrix.TRS(V.XYZ(-0.5f, 0.05f, -0.5f), Quat.LookDir(1, 0, 1), 2);
	string title       = "Microphone";

	Pose     windowPose = new Pose(0.5f,0,-0.5f, Quat.LookDir(-1,0,1));
	string[] devices;
	string   activeDevice;
	Sprite   micSprite;
	Material micMaterial;
	float[]  micBuffer = new float[400];
	float    micIntensity     = 0;
	float    micIntensityDest = 0;

	public void Initialize()
	{
		micSprite = Sprite.FromFile("mic_icon.png", SpriteType.Single);
		micMaterial = Default.MaterialUnlit.Copy();
		micMaterial.Transparency = Transparency.Blend;

		devices = Microphone.GetDevices();
		Microphone.Start();
	}

	public void Shutdown()
	{
		Microphone.Stop();
	}

	void SetMic(string name)
	{
		activeDevice = name;
		Microphone.Start(name);
	}
	public void Update()
	{
		Vec2 size = V.XY(0.25f, UI.LineHeight);

		UI.WindowBegin("Available Microphones:", ref windowPose);
		if (UI.Radio("Default", activeDevice == null, size)) SetMic(null);
		foreach(string device in devices)
			if (UI.Radio(device, activeDevice == device, size)) SetMic(device);
		UI.WindowEnd();

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


			Log.Info(micIntensity.ToString());
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