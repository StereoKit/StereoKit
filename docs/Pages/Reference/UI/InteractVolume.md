---
layout: default
title: UI.InteractVolume
description: This watches a volume of space for pinch interaction events! If a hand is inside the space indicated by the bounds, this function will return that hand's pinch state, as well as indicate which hand did it through the out parameter.  Note that since this only provides the hand's pinch state, it won't give you JustActive and JustInactive notifications for when the hand enters or leaves the volume.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).InteractVolume

<div class='signature' markdown='1'>
static [BtnState]({{site.url}}/Pages/Reference/BtnState.html) InteractVolume([Bounds]({{site.url}}/Pages/Reference/Bounds.html) bounds, Handed& hand)
</div>

|  |  |
|--|--|
|[Bounds]({{site.url}}/Pages/Reference/Bounds.html) bounds|A UI hierarchy space bounding volume.|
|Handed& hand|This will be the last hand that provides a              pinch state within this volume. That means that if both hands are             pinching in this volume, it will provide the Right hand.|
|RETURNS: [BtnState]({{site.url}}/Pages/Reference/BtnState.html)|This will be the pinch state of the last hand that provides a pinch state within this volume. That means that if both hands are pinching in this volume, it will provide the pinch state of the Right hand.|

This watches a volume of space for pinch interaction
events! If a hand is inside the space indicated by the bounds,
this function will return that hand's pinch state, as well as
indicate which hand did it through the out parameter.

Note that since this only provides the hand's pinch state, it
won't give you JustActive and JustInactive notifications for
when the hand enters or leaves the volume.




## Examples

This code will draw an axis at the index finger's location when
the user pinches while inside an InteractVolume.

![UI.InteractVolume]({{site.screen_url}}/InteractVolume.jpg)

```csharp
// Draw a transparent volume so the user can see this space
Vec3  volumeAt   = new Vec3(0,0.2f,-0.4f);
float volumeSize = 0.2f;
Default.MeshCube.Draw(Default.MaterialUIBox, Matrix.TS(volumeAt, volumeSize));

BtnState volumeState = UI.InteractVolume(new Bounds(volumeAt, Vec3.One*volumeSize), out Handed hand);
if (volumeState != BtnState.Inactive)
{
	// If it just changed interaction state, make it jump in size
	float scale = volumeState.IsChanged()
		? 0.1f
		: 0.05f;
	Lines.AddAxis(Input.Hand(hand)[FingerId.Index, JointId.Tip].Pose, scale);
}
```

