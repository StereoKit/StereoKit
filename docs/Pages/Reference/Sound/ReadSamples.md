---
layout: default
title: Sound.ReadSamples
description: This will read samples from the sound stream, starting from the first unread sample. Check UnreadSamples for how many samples are available to read.
---
# [Sound]({{site.url}}/Pages/Reference/Sound.html).ReadSamples

<div class='signature' markdown='1'>
```csharp
int ReadSamples(Single[]& samples)
```
This will read samples from the sound stream, starting
from the first unread sample. Check UnreadSamples for how many
samples are available to read.
</div>

|  |  |
|--|--|
|Single[]& samples|A pre-allocated buffer to read the samples             into! This function will stop reading when this buffer is full,              or when it runs out of unread samples.|
|RETURNS: int|Returns the number of samples that were read from the stream's buffer and written to the provided sample buffer.|





## Examples

### Getting streaming sound intensity
This example shows how to read data from a Sound stream such as the
microphone! In this case, we're just finding the average 'intensity'
of the audio, and returning it as a value approximately between 0 and 1.
Microphone.Start() should be called before this example :)
```csharp
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

