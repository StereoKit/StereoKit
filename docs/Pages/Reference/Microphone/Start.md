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

