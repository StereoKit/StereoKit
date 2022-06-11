---
layout: default
title: Using QR Codes
description: QR codes are a super fast and easy way to locate an object, provide information from the environment, or localize two devices to the same coordinate space! ...
---

# Using QR Codes

QR codes are a super fast and easy way to locate an object,
provide information from the environment, or `localize` two
devices to the same coordinate space! HoloLens 2 and WMR headsets
have a really convenient way to grab and use this data. They can use
the tracking cameras of the device, at the driver level to provide
QR codes from the environment, pretty much for free!

The only caveat is that tracking cameras are lower resolution, so
they need big QR codes, or to be very close to the codes. They also
only update around 2 times a second. But if that suits your needs?
Then you're in luck!

## Pre-Requisites

QR code support is not built directly in to StereoKit, but
it is quite trivial to implement! For this, we use the
[Microsoft MixedReality QR code library](https://docs.microsoft.com/en-us/windows/mixed-reality/qr-code-tracking)
through its NuGet package. This will require a UWP StereoKit
project, and the Webcam capability in the project's
.appxmanifest file.

So! That's the pre-reqs for this guide!

 - A StereoKit UWP project.
 - The [NuGet package](https://www.nuget.org/Packages/Microsoft.MixedReality.QR).
 - Enable the `Webcam` capability in the .appxmanifest file.

Then in your code, you'll be able to add this using
statement and get access to the `QRCodeWatcher`, the main
interface to the QR code functionality.
```csharp
using Microsoft.MixedReality.QR;
```
## Code

For code, we'll start with our own representation of
what a QR code means. Nothing fancy, we just want to
show the orientation and contents of each code! So, pose,
size, and data as text.

We'll also include a function to convert the WMR QR code into
our own. The only fancy stuff happening here is grabbing the
Pose! The `SpatialGraphNodeId` contains a pose, but it's in
UWPs coordinate space. `Pose.FromSpatialNode` is a bridge
function that will convert from UWP's coordinates into our own.
```csharp
struct QRData
{ 
	public Pose   pose;
	public float  size;
	public string text;
	public static QRData FromCode(QRCode qr)
	{
		QRData result = new QRData();
		// It's not unusual for this to fail to find a pose, especially on
		// the first frame it's been seen.
		World.FromSpatialNode(qr.SpatialGraphNodeId, out result.pose);
		result.size = qr.PhysicalSideLength;
		result.text = qr.Data == null ? "" : qr.Data;
		return result;
	}
}
```
Ok, cool! Now here's the data we'll be tracking for this demo,
the `QRCodeWatcher` is the object that'll provide us QR data,
`watcherStart` will let us filter out QR codes from other sesions,
and `poses` is our list of unique QR codes that we can iterate through
and draw.
```csharp
QRCodeWatcher watcher;
DateTime      watcherStart;
Dictionary<Guid, QRData> poses = new Dictionary<Guid, QRData>();
```
Initialization is just a matter of asking for permission, and then
hooking up to the `QRCodeWatcher`'s events. `QRCodeWatcher.RequestAccessAsync`
is an async call, so you could re-arrange this code to be non-blocking!

You'll also notice there's some code here for filtering out QR codes.
The default behavior for the QR code library is to provide all QR
codes that it knows about, and that includes ones that were found
before the session began. We don't need that, so we're ignoring those.
```csharp
public void Initialize()
{
	// Ask for permission to use the QR code tracking system
	var status = QRCodeWatcher.RequestAccessAsync().Result;
	if (status != QRCodeWatcherAccessStatus.Allowed)
		return;

	// Set up the watcher, and listen for QR code events.
	watcherStart = DateTime.Now;
	watcher      = new QRCodeWatcher();
	watcher.Added   += (o, qr) => {
		// QRCodeWatcher will provide QR codes from before session start,
		// so we often want to filter those out.
		if (qr.Code.LastDetectedTime > watcherStart) 
			poses.Add(qr.Code.Id, QRData.FromCode(qr.Code)); 
	};
	watcher.Updated += (o, qr) => poses[qr.Code.Id] = QRData.FromCode(qr.Code);
	watcher.Removed += (o, qr) => poses.Remove(qr.Code.Id);
	watcher.Start();
}

// For shutdown, we just need to stop the watcher
public void Shutdown() => watcher?.Stop();

```
Now all we need to do is show the QR codes! In this case,
we're just displaying an axis widget, and the contents of
the QR code as text.

With the text, all we're doing is squeezing the text into
the bounds of the QR code, and shifting it to be a little
forward, in front of the code!
```csharp
public void Update()
{
	foreach(QRData d in poses.Values)
	{ 
		Lines.AddAxis(d.pose, d.size);
		Text .Add(
			d.text, 
			d.pose.ToMatrix(),
			Vec2.One * d.size,
			TextFit.Squeeze,
			TextAlign.XLeft | TextAlign.YTop,
			TextAlign.Center,
			d.size, d.size);
	}
}
```
And that's all there is to it! You can find all this code
in context [here on Github](https://github.com/StereoKit/StereoKit/blob/master/Examples/StereoKitTest/Demos/DemoQRCode.cs).

