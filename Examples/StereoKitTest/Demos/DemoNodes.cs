using StereoKit;
using System;

class DemoNodes : ITest
{
	Matrix descPose    = Matrix.TR (-0.5f, 0, -0.5f, Quat.LookDir(1,0,1));
	string description = "ModelNode API lets...";
	Matrix titlePose   = Matrix.TRS(V.XYZ(-0.5f, 0.05f, -0.5f), Quat.LookDir(1, 0, 1), 2);
	string title       = "Model Nodes";

	Model radio     = Model.FromFile("Radio.glb");
	Pose  radioPose = new Pose(0.5f, -0.25f, -0.5f, Quat.LookDir(-1, 0, 1));
	Pose  radioPowerPose;
	Pose  radioSpeakerPose;
	Pose  radioInterfacePose;
	Vec2  radioInterfaceSize;
	Pose  radioDisplayPose;
	Vec2  radioDisplaySize;

	float       radioFreq = 500;
	float       radioVolume = 0.5f;
	bool        radioPlay = false;
	LinePoint[] radioFreqLine = new LinePoint[256];
	Sound       radioSound;
	SoundInst   radioSoundInst;
	float[]     radioSoundBuffer = new float[(int)(0.1f*48000)];
	double      radioSoundBufferT = 0;

	public void Initialize()
	{
		radioPowerPose     = radio.FindNode("Power").ModelTransform.Pose;
		radioSpeakerPose   = radio.FindNode("Speaker").ModelTransform.Pose;
		radioDisplayPose   = radio.FindNode("DisplayStart").ModelTransform.Pose;
		radioDisplaySize   = radio.FindNode("DisplayEnd").LocalTransform.Translation.XY;
		radioDisplaySize   = V.XY(Math.Abs(radioDisplaySize.x), Math.Abs(radioDisplaySize.y));
		radioInterfacePose = radio.FindNode("InterfaceStart").ModelTransform.Pose;
		radioInterfaceSize = radio.FindNode("InterfaceEnd").LocalTransform.Translation.XY;
		radioInterfaceSize = V.XY(Math.Abs(radioInterfaceSize.x), Math.Abs(radioInterfaceSize.y));

		UpdateLine(radioFreqLine, radioFreq);

		radioSound     = Sound.CreateStream(1);
		radioSoundInst = radioSound.Play(Vec3.Zero);
	}

	public void Update()
	{
		UI.HandleBegin("Radio", ref radioPose, radio.Bounds);
		{
			radio.Draw(Matrix.Identity);
			UI.PushSurface(radioInterfacePose, default, radioInterfaceSize);
			{
				if (UI.HSlider("Freq", ref radioFreq, 0, 1000, 0, 0, UIConfirm.Pinch))
					UpdateLine(radioFreqLine, radioFreq);
			}
			UI.PopSurface();

			UI.PushSurface(radioPowerPose);
			{
				UI.Toggle("Power", ref radioPlay);
			}
			UI.PopSurface();

			radioSoundInst.Position = Hierarchy.ToWorld(radioSpeakerPose.position);

			Hierarchy.Push(radioDisplayPose.ToMatrix(V.XYZ(radioDisplaySize.x, radioDisplaySize.y, 1)));
			Lines.Add(radioFreqLine);
			Hierarchy.Pop();
		}
		UI.HandleEnd();

		if (radioSound.UnreadSamples < radioSoundBuffer.Length && radioPlay)
		{
			for (int i = 0; i < radioSoundBuffer.Length; i++)
			{
				radioSoundBuffer[i] = (float)Math.Sin(radioSoundBufferT) * radioVolume;
				radioSoundBufferT += (1 / 48000.0) * radioFreq;
			}
			radioSound.WriteSamples(radioSoundBuffer);
		}

		Text.Add(title, titlePose);
		Text.Add(description, descPose, V.XY(0.4f, 0), TextFit.Wrap, TextAlign.TopCenter, TextAlign.TopLeft);
	}

	void UpdateLine(LinePoint[] line, float freq)
	{
		for (int i = 0; i < line.Length; i++)
		{
			float pct = i / (line.Length - 1.0f);
			line[i].color     = Color32.White;
			line[i].thickness = 2 * U.mm;
			line[i].pt        = new Vec3(-pct, MathF.Sin((pct-0.5f)*freq*0.1f), 0);
		}
	}

	public void Shutdown()
	{
		radioSoundInst.Stop();
	}
}