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

