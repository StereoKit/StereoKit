using StereoKit;
using System.Collections.Generic;
using System.Linq;

class DocRecordAudio : ITest
{
	List<float> soundSamples = new List<float>();
	float[]     sampleBuffer = new float[1];
	Sound       recordedSound;

	Pose windowPose = new Pose(0.5f, 0, -0.5f, Quat.LookDir(-1, 0, 1));

	public void Update()
	{
		if (Microphone.IsRecording)
		{
			if (sampleBuffer.Length < Microphone.Sound.UnreadSamples)
				sampleBuffer = new float[Microphone.Sound.UnreadSamples];

			int read = Microphone.Sound.ReadSamples(ref sampleBuffer);
			soundSamples.AddRange(sampleBuffer.Take(read));
		}

		UI.WindowBegin("Recording!", ref windowPose);
		bool recording = Microphone.IsRecording;
		if (UI.Toggle("Record", ref recording))
		{
			if (recording)
			{
				soundSamples.Clear();
				Microphone.Start();
			}
			else
			{
				recordedSound = Sound.FromSamples(soundSamples.ToArray());
				Microphone.Stop();
				
			}
		}
		UI.PushEnabled(recordedSound != null);
		if (UI.Button("Play Recording"))
		{
			recordedSound.Play(Hierarchy.ToWorld( UI.LayoutLast.center ));
		}
		UI.PopEnabled();
		UI.WindowEnd();
	}
	public void Shutdown() => Microphone.Stop();
	public void Initialize() { }
}