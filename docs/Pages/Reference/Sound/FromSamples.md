---
layout: default
title: Sound.FromSamples
description: This function will create a sound from an array of samples. Values should range from -1 to +1, and there should be 48,000 values per second of audio.
---
# [Sound]({{site.url}}/Pages/Reference/Sound.html).FromSamples

<div class='signature' markdown='1'>
```csharp
static Sound FromSamples(Single[] samplesAt48000s)
```
This function will create a sound from an array of
samples. Values should range from -1 to +1, and there should be
48,000 values per second of audio.
</div>

|  |  |
|--|--|
|Single[] samplesAt48000s|Values should range from -1 to +1,              and there should be 48,000 per second of audio.|
|RETURNS: [Sound]({{site.url}}/Pages/Reference/Sound.html)|Returns a sound effect from the samples provided! Or null if something went wrong.|





## Examples

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
### Generating a sound via samples
Making a procedural sound is pretty straightforward! Here's
an example of building a 500ms sound from two frequencies of
sin wave.
```csharp
float[] samples = new float[(int)(48000*0.5f)];
for (int i = 0; i < samples.Length; i++)
{
	float t = i/48000.0f;
	float band1 = SKMath.Sin(t * 523.25f * SKMath.Tau); // a 'C' tone
	float band2 = SKMath.Sin(t * 659.25f * SKMath.Tau); // an 'E' tone
	const float volume = 0.1f;
	samples[i] = (band1 * 0.6f + band2 * 0.4f) * volume;
}
Sound sampleSound = Sound.FromSamples(samples);
sampleSound.Play(Vec3.Zero);
```

