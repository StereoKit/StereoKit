---
layout: default
title: Sound.UnreadSamples
description: This is the maximum number of samples in the sound that are currently available for reading via ReadSamples! ReadSamples will reduce this number by the amount of samples read.  This is only really valid for Stream sounds, all other sound types will just return 0.
---
# [Sound]({{site.url}}/Pages/Reference/Sound.html).UnreadSamples

<div class='signature' markdown='1'>
int UnreadSamples{ get }
</div>

## Description
This is the maximum number of samples in the sound that
are currently available for reading via ReadSamples! ReadSamples
will reduce this number by the amount of samples read.

This is only really valid for Stream sounds, all other sound
types will just return 0.


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

