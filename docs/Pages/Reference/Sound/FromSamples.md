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

