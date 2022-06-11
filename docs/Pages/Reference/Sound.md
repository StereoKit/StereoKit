---
layout: default
title: Sound
description: This class represents a sound effect! Excellent for blips and bloops and little clips that you might play around your scene. Not great for long streams of audio like you might see in a podcast. Right now, this supports .wav, .mp3, and procedurally generated noises!  On HoloLens 2, sounds are automatically processed on the HPU, freeing up the CPU for more of your app's code. To simulate this same effect on your development PC, you need to enable spatial sound on your audio endpoint. To do this, right click the speaker icon in your system tray, navigate to "Spatial sound", and choose "Windows Sonic for Headphones." For more information, visit https.//docs.microsoft.com/en-us/windows/win32/coreaudio/spatial-sound
---
# class Sound

This class represents a sound effect! Excellent for blips
and bloops and little clips that you might play around your scene.
Not great for long streams of audio like you might see in a podcast.
Right now, this supports .wav, .mp3, and procedurally generated
noises!

On HoloLens 2, sounds are automatically processed on the HPU, freeing
up the CPU for more of your app's code. To simulate this same effect
on your development PC, you need to enable spatial sound on your
audio endpoint. To do this, right click the speaker icon in your
system tray, navigate to "Spatial sound", and choose "Windows Sonic
for Headphones." For more information, visit
https://docs.microsoft.com/en-us/windows/win32/coreaudio/spatial-sound

## Instance Fields and Properties

|  |  |
|--|--|
|int [CursorSamples]({{site.url}}/Pages/Reference/Sound/CursorSamples.html)|This is the current position of the playback cursor, measured in samples from the start of the audio data.|
|float [Duration]({{site.url}}/Pages/Reference/Sound/Duration.html)|This will return the total length of the sound in seconds.|
|int [TotalSamples]({{site.url}}/Pages/Reference/Sound/TotalSamples.html)|This will return the total number of audio samples used by the sound! StereoKit currently uses 48,000 samples per second for all audio.|
|int [UnreadSamples]({{site.url}}/Pages/Reference/Sound/UnreadSamples.html)|This is the maximum number of samples in the sound that are currently available for reading via ReadSamples! ReadSamples will reduce this number by the amount of samples read.  This is only really valid for Stream sounds, all other sound types will just return 0.|

## Instance Methods

|  |  |
|--|--|
|[Play]({{site.url}}/Pages/Reference/Sound/Play.html)|Plays the sound at the 3D location specified, using the volume parameter as an additional volume control option! Sound volume falls off from 3D location, and can also indicate direction and location through spatial audio cues. So make sure the position is where you want people to think it's from! Currently, if this sound is playing somewhere else, it'll be canceled, and moved to this location.|
|[ReadSamples]({{site.url}}/Pages/Reference/Sound/ReadSamples.html)|This will read samples from the sound stream, starting from the first unread sample. Check UnreadSamples for how many samples are available to read.|
|[WriteSamples]({{site.url}}/Pages/Reference/Sound/WriteSamples.html)|Only works if this Sound is a stream type! This writes a number of audio samples to the sample buffer, and samples should be between -1 and +1. Streams are stored as ring buffers of a fixed size, so writing beyond the capacity of the ring buffer will overwrite the oldest samples.  StereoKit uses 48,000 samples per second of audio.|

## Static Fields and Properties

|  |  |
|--|--|
|[Sound]({{site.url}}/Pages/Reference/Sound.html) [Click]({{site.url}}/Pages/Reference/Sound/Click.html)|A default click sound that lasts for 300ms. It's a procedurally generated sound based on a mouse press, with extra low frequencies in it.|
|[Sound]({{site.url}}/Pages/Reference/Sound.html) [Unclick]({{site.url}}/Pages/Reference/Sound/Unclick.html)|A default click sound that lasts for 300ms. It's a procedurally generated sound based on a mouse release, with extra low frequencies in it.|

## Static Methods

|  |  |
|--|--|
|[CreateStream]({{site.url}}/Pages/Reference/Sound/CreateStream.html)|Create a sound used for streaming audio in or out! This is useful for things like reading from a microphone stream, or playing audio from a source streaming over the network, or even procedural sounds that are generated on the fly!  Use stream sounds with the WriteSamples and ReadSamples functions.|
|[Find]({{site.url}}/Pages/Reference/Sound/Find.html)|Looks for a Sound asset that's already loaded, matching the given id!|
|[FromFile]({{site.url}}/Pages/Reference/Sound/FromFile.html)|Loads a sound effect from file! Currently, StereoKit supports .wav and .mp3 files. Audio is converted to mono.|
|[FromSamples]({{site.url}}/Pages/Reference/Sound/FromSamples.html)|This function will create a sound from an array of samples. Values should range from -1 to +1, and there should be 48,000 values per second of audio.|
|[Generate]({{site.url}}/Pages/Reference/Sound/Generate.html)|This function will generate a sound from a function you provide! The function is called once for each sample in the duration. As an example, it may be called 48,000 times for each second of duration.|

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

### Basic usage
```csharp
Sound sound = Sound.FromFile("BlipNoise.wav");
sound.Play(Vec3.Zero);
```

### Generating a sound via generator
Making a procedural sound is pretty straightforward! Here's
an example of building a 500ms sound from two frequencies of
sin wave.
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

