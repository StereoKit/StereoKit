---
layout: default
title: UI.VolumeAt
description: An invisible volume that will trigger when a finger enters it!
---
# [UI]({{site.url}}/Pages/Reference/UI.html).VolumeAt

<div class='signature' markdown='1'>
```csharp
static bool VolumeAt(string id, Bounds bounds)
```
An invisible volume that will trigger when a finger enters
it!
</div>

|  |  |
|--|--|
|string id|An id for tracking element state. MUST be unique             within current hierarchy.|
|[Bounds]({{site.url}}/Pages/Reference/Bounds.html) bounds|Size and position of the volume, relative to             the current Hierarchy.|
|RETURNS: bool|True on the first frame a finger has entered the volume, false otherwise.|





## Examples

This code will draw an axis at the index finger's location when
the user pinches while inside a VolumeAt.

![UI.InteractVolume]({{site.screen_url}}/InteractVolume.jpg)

```csharp
// Draw a transparent volume so the user can see this space
Vec3  volumeAt   = new Vec3(0,0.2f,-0.4f);
float volumeSize = 0.2f;
Default.MeshCube.Draw(Default.MaterialUIBox, Matrix.TS(volumeAt, volumeSize));

BtnState volumeState = UI.VolumeAt("Volume", new Bounds(volumeAt, Vec3.One*volumeSize), UIConfirm.Pinch, out Handed hand);
if (volumeState != BtnState.Inactive)
{
	// If it just changed interaction state, make it jump in size
	float scale = volumeState.IsChanged()
		? 0.1f
		: 0.05f;
	Lines.AddAxis(Input.Hand(hand)[FingerId.Index, JointId.Tip].Pose, scale);
}
```

