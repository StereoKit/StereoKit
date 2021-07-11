---
layout: default
title: Input.Eyes
description: If the device has eye tracking hardware and the app has permission to use it, then this is the most recently tracked eye pose. Check Input.EyesTracked to see if the pose is up-to date, or if it's a leftover!  You can also check SK.System.eyeTrackingPresent to see if the hardware is capable of providing eye tracking.  On Flatscreen when the MR sim is still enabled, then eyes are emulated using the cursor position when the user holds down Alt.
---
# [Input]({{site.url}}/Pages/Reference/Input.html).Eyes

<div class='signature' markdown='1'>
static [Pose]({{site.url}}/Pages/Reference/Pose.html) Eyes{ get }
</div>

## Description
If the device has eye tracking hardware and the app has
permission to use it, then this is the most recently tracked eye
pose. Check `Input.EyesTracked` to see if the pose is up-to date,
or if it's a leftover!

You can also check `SK.System.eyeTrackingPresent` to see if the
hardware is capable of providing eye tracking.

On Flatscreen when the MR sim is still enabled, then eyes are
emulated using the cursor position when the user holds down Alt.


## Examples

```csharp
if (Input.EyesTracked.IsActive())
{
	// Intersect the eye Ray with a floor plane
	Plane plane = new Plane(Vec3.Zero, Vec3.Up);
	if (Input.Eyes.Ray.Intersect(plane, out Vec3 at))
	{
		Default.MeshSphere.Draw(Default.Material, Matrix.TS(at, .05f));
	}
}
```

