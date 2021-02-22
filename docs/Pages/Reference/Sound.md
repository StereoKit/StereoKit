---
layout: default
title: Sound
description: This class represents a sound effect! Excellent for blips and bloops and little clips that you might play around your scene. Not great for long streams of audio like you might see in a podcast. Right now, this only supports .wav files, and procedurally generated noises!  On HoloLens 2, sounds are automatically processed on the HPU, freeing up the CPU for more of your app's code. To simulate this same effect on your development PC, you need to enable spatial sound on your audio endpoint. To do this, right click the speaker icon in your system tray, navigate to "Spatial sound", and choose "Windows Sonic for Headphones." For more information, visit https.//docs.microsoft.com/en-us/windows/win32/coreaudio/spatial-sound
---
# Sound

This class represents a sound effect! Excellent for blips
and bloops and little clips that you might play around your scene.
Not great for long streams of audio like you might see in a podcast.
Right now, this only supports .wav files, and procedurally generated
noises!

On HoloLens 2, sounds are automatically processed on the HPU, freeing
up the CPU for more of your app's code. To simulate this same effect
on your development PC, you need to enable spatial sound on your
audio endpoint. To do this, right click the speaker icon in your
system tray, navigate to "Spatial sound", and choose "Windows Sonic
for Headphones." For more information, visit
https://docs.microsoft.com/en-us/windows/win32/coreaudio/spatial-sound



## Instance Methods

|  |  |
|--|--|
|[Play]({{site.url}}/Pages/Reference/Sound/Play.html)|Plays the sound at the 3D location specified, using the volume parameter as an additional volume control option! Sound volume falls off from 3D location, and can also indicate direction and location through spatial audio cues. So make sure the position is where you want people to think it's from! Currently, if this sound is playing somewhere else, it'll be cancelled, and moved to this location.|



## Static Methods

|  |  |
|--|--|
|[Find]({{site.url}}/Pages/Reference/Sound/Find.html)|Looks for a Sound asset that's already loaded, matching the given id!|
|[FromFile]({{site.url}}/Pages/Reference/Sound/FromFile.html)|Loads a sound effect from file! Currently, StereoKit only supports .wav files. Audio is converted to mono.|
|[Generate]({{site.url}}/Pages/Reference/Sound/Generate.html)|This function will generate a sound from a function you provide! The function is called once for each sample in the duration. As an example, it may be called 48,000 times for each second of duration.|


## Examples

### Basic usage
```csharp
Sound sound = Sound.FromFile("BlipNoise.wav");
sound.Play(Vec3.Zero);
```

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

