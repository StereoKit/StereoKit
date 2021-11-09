---
layout: default
title: Microphone.GetDevices
description: Constructs a list of valid Microphone devices attached to the system. These names can be passed into Start to select a specific device to record from. It's recommended to cache this list if you're using it frequently, as this list is constructed each time you call it.  It's good to note that a user might occasionally plug or unplug microphone devices from their system, so this list may occasionally change.
---
# [Microphone]({{site.url}}/Pages/Reference/Microphone.html).GetDevices

<div class='signature' markdown='1'>
```csharp
static String[] GetDevices()
```
Constructs a list of valid Microphone devices attached
to the system. These names can be passed into Start to select
a specific device to record from. It's recommended to cache this
list if you're using it frequently, as this list is constructed
each time you call it.

It's good to note that a user might occasionally plug or unplug
microphone devices from their system, so this list may
occasionally change.
</div>

|  |  |
|--|--|
|RETURNS: String[]|List of human readable microphone device names.|





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

