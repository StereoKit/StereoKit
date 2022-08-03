---
layout: default
title: Microphone.Start
description: This begins recording audio from the Microphone! Audio is stored in Microphone.Sound as a stream of audio. If the Microphone is already recording with a different device, it will stop the previous recording and start again with the new device.  If null is provided as the device, then they system's default input device will be used. Some systems may not provide access to devices other than the system's default.
---
# [Microphone]({{site.url}}/Pages/Reference/Microphone.html).Start

<div class='signature' markdown='1'>
```csharp
static bool Start(string deviceName)
```
This begins recording audio from the Microphone! Audio
is stored in Microphone.Sound as a stream of audio. If the
Microphone is already recording with a different device, it will
stop the previous recording and start again with the new device.

If null is provided as the device, then they system's default
input device will be used. Some systems may not provide access
to devices other than the system's default.
</div>

|  |  |
|--|--|
|string deviceName|The name of the microphone device to             use, as seen in the GetDevices list. null will use the system's             default device preference.|
|RETURNS: bool|True if recording started successfully, false for failure. This could fail if the app does not have mic permissions, or if the deviceName is for a mic that has since been unplugged.|





## Examples

### Choosing a microphone device
While generally you'll prefer to use the default device, it can be
nice to allow users to pick which mic they're using! This is
especially important on PC, where users may have complicated or
interesting setups.

![Microphone device selection window]({{site.screen_url}}/MicrophoneSelector.jpg)

This sample is a very simple window that allows users to start
recording with a device other than the default. NOTE: this example
is designed with the assumption that Microphone.Start() has been
called already.
```csharp
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
```
### Recording Audio Snippets
A common use case for the microphone would be to record a snippet of
audio! This demo is a window that will read data from the Microphone,
and use that to create a sound for playback.

![Audio recording window]({{site.screen_url}}/RecordAudioSnippet.jpg)
```csharp
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
```

