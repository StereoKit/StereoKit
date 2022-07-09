#if !WINDOWS_UWP
using System.Collections.Generic;
using StereoKit;

class DemoRecordMic : ITest
{
	Matrix descPose    = Matrix.TR (-0.5f, 0, -0.5f, Quat.LookDir(1,0,1));
	string description = "A common use case for the microphone would be to record a snippet of audio! This demo shows reading data from the Microphone, and using that to create a sound for playback.";
	Matrix titlePose   = Matrix.TRS(V.XYZ(-0.5f, 0.05f, -0.5f), Quat.LookDir(1, 0, 1), 2);
	string title       = "Record Mic";

	/// :CodeSample: Microphone Microphone.Start Sound.ReadSamples Sound.FromSamples 
	/// ### Recording Audio Snippets
	/// A common use case for the microphone would be to record a snippet of
	/// audio! This demo is a window that will read data from the Microphone,
	/// and use that to create a sound for playback.
	///
	/// ![Audio recording window]({{site.screen_url}}/RecordAudioSnippet.jpg)
	Sound       recordedSound   = null;
	List<float> recordedData    = new List<float>();
	float[]     sampleBuffer    = null;
	bool        recording       = false;
	Pose        recordingWindow = new Pose(0.5f, 0, -0.5f, Quat.LookDir(-1, 0, 1));

	void RecordAudio()
	{
		UI.WindowBegin("Recording Panel", ref recordingWindow);

		// This code will begin a new recording, or finish an existing
		// recording!
		if (UI.Toggle("Record!", ref recording))
		{
			if (recording)
			{
				// Clear out our data, and start up the mic!
				recordedData.Clear();
				recording = Microphone.Start();
				if (!recording)
					Log.Warn("Recording failed to start!");
			}
			else
			{
				// Stop the mic, and pour our recorded samples into a new Sound
				Microphone.Stop();
				recordedSound = Sound.FromSamples(recordedData.ToArray());
			}
		}

		// If the mic is recording, every frame we'll want to grab all the data
		// from the Microphone's audio stream, and store it until we can make
		// a complete sound from it.
		if (Microphone.IsRecording)
		{
			if (sampleBuffer == null || sampleBuffer.Length < Microphone.Sound.UnreadSamples)
				sampleBuffer = new float[Microphone.Sound.UnreadSamples];
			int read = Microphone.Sound.ReadSamples(ref sampleBuffer);
			recordedData.AddRange(sampleBuffer[0..read]);
		}

		// Let the user know the current status of our recording code.
		UI.SameLine();
		if      (Microphone.IsRecording) UI.Label("recording...");
		else if (recordedSound != null)  UI.Label($"{recordedSound.Duration:0.#}s");
		else                             UI.Label("...");

		// If we have a recording, give the user a button that'll play it back!
		UI.PushEnabled(recordedSound != null);
		if (UI.Button("Play Recording"))
			recordedSound.Play(recordingWindow.position);
		UI.PopEnabled();
		
		UI.WindowEnd();
	}
	/// :End:

	public void Initialize() => Tests.RunForFrames(2);
	public void Shutdown() => Microphone.Stop();
	public void Update()
	{
		ShowMicDeviceWindow();
		RecordAudio();

		Tests.Screenshot("RecordAudioSnippet.jpg", 1, 400, 400, 90, recordingWindow.position + V.XYZ(-0.08f, -0.04f, 0.08f), recordingWindow.position - V.XYZ(0, 0.04f, 0));

		Text.Add(title, titlePose);
		Text.Add(description, descPose, V.XY(0.4f, 0), TextFit.Wrap, TextAlign.TopCenter, TextAlign.TopLeft);
	}

	Pose     micSelectPose   = new Pose(0.7f, 0, -0.3f, Quat.LookDir(-1, 0, 1));
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
		}
		foreach (string device in micDevices)
		{
			if (UI.Radio(device, micDeviceActive == device, size))
			{
				micDeviceActive = device;
			}
		}

		UI.WindowEnd();
	}
}
#endif