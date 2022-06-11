---
layout: default
title: Microphone
description: This class provides access to the hardware's microphone, and stores it in a Sound stream. Start and Stop recording, and check the Sound property for the results! Remember to ensure your application has microphone permissions enabled!
---
# static class Microphone

This class provides access to the hardware's microphone, and
stores it in a Sound stream. Start and Stop recording, and check the
Sound property for the results! Remember to ensure your application
has microphone permissions enabled!

## Static Fields and Properties

|  |  |
|--|--|
|bool [IsRecording]({{site.url}}/Pages/Reference/Microphone/IsRecording.html)|Tells if the Microphone is currently recording audio.|
|[Sound]({{site.url}}/Pages/Reference/Sound.html) [Sound]({{site.url}}/Pages/Reference/Microphone/Sound.html)|This is the sound stream of the Microphone when it is recording. This Asset is created the first time it is accessed via this property, or during Start, and will persist. It is re-used for the Microphone stream if you start/stop/switch devices.|

## Static Methods

|  |  |
|--|--|
|[GetDevices]({{site.url}}/Pages/Reference/Microphone/GetDevices.html)|Constructs a list of valid Microphone devices attached to the system. These names can be passed into Start to select a specific device to record from. It's recommended to cache this list if you're using it frequently, as this list is constructed each time you call it.  It's good to note that a user might occasionally plug or unplug microphone devices from their system, so this list may occasionally change.|
|[Start]({{site.url}}/Pages/Reference/Microphone/Start.html)|This begins recording audio from the Microphone! Audio is stored in Microphone.Sound as a stream of audio. If the Microphone is already recording with a different device, it will stop the previous recording and start again with the new device.  If null is provided as the device, then they system's default input device will be used. Some systems may not provide access to devices other than the system's default.|
|[Stop]({{site.url}}/Pages/Reference/Microphone/Stop.html)|If the Microphone is recording, this will stop it.|

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

