---
layout: default
title: Input.EyesTracked
description: If eye hardware is available and app has permission, then this is the tracking state of the eyes. Eyes may move out of bounds, hardware may fail to detect eyes, or who knows what else!  On Flatscreen when MR sim is still enabled, this will report whether the user is simulating eye input with the Alt key.  **Permissions** - For UWP apps, permissions for eye tracking can be found in the project's .appxmanifest file under Capabilities->Gaze Input. - For Xamarin apps, you may need to add an entry to your AndroidManifest.xml, refer to your device's documentation for specifics.
---
# [Input]({{site.url}}/Pages/Reference/Input.html).EyesTracked

<div class='signature' markdown='1'>
static [BtnState]({{site.url}}/Pages/Reference/BtnState.html) EyesTracked{ get }
</div>

## Description
If eye hardware is available and app has permission,
then this is the tracking state of the eyes. Eyes may move out of
bounds, hardware may fail to detect eyes, or who knows what else!

On Flatscreen when MR sim is still enabled, this will report
whether the user is simulating eye input with the Alt key.

**Permissions**
- For UWP apps, permissions for eye tracking can be found in the project's .appxmanifest file under Capabilities->Gaze Input.
- For Xamarin apps, you may need to add an entry to your AndroidManifest.xml, refer to your device's documentation for specifics.


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

