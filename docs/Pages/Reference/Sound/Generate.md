---
layout: default
title: Sound.Generate
description: This function will generate a sound from a function you provide! The function is called once for each sample in the duration. As an example, it may be called 48,000 times for each second of duration.
---
# [Sound]({{site.url}}/Pages/Reference/Sound.html).Generate

<div class='signature' markdown='1'>
```csharp
static Sound Generate(AudioGenerator generator, float duration)
```
This function will generate a sound from a function you provide! The
function is called once for each sample in the duration. As an example, it
may be called 48,000 times for each second of duration.
</div>

|  |  |
|--|--|
|AudioGenerator generator|This function takes a time value as an argument, which             will range from 0-duration, and should return a value from -1 - +1 representing             the audio wave at that point in time.|
|float duration|In seconds, how long should the sound be?|
|RETURNS: [Sound]({{site.url}}/Pages/Reference/Sound.html)|Returns a generated sound effect! Or null if something went wrong.|





## Examples

### Generating a sound
Making a procedural sound is pretty straightforward! Here's
an example of building a sound from two frequencies of sin
wave.
```csharp
Sound genSound = Sound.Generate((t) =>
{
	float band1 = SKMath.Sin(t * 523.25f * SKMath.Tau); // a 'C' tone
	float band2 = SKMath.Sin(t * 659.25f * SKMath.Tau); // an 'E' tone
	const float volume = 0.1f;
	return (band1*0.6f + band2*0.4f) * volume;
}, 0.5f);
genSound.Play(Vec3.Zero);
```

