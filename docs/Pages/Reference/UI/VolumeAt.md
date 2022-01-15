---
layout: default
title: UI.VolumeAt
description: A volume for helping to build one handed interactions. This checks for the presence of a hand inside the bounds, and if found, return that hand along with activation and focus information defined by the interactType.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).VolumeAt

<div class='signature' markdown='1'>
```csharp
static BtnState VolumeAt(string id, Bounds bounds, UIConfirm interactType, Handed& hand, BtnState& focusState)
```
A volume for helping to build one handed interactions.
This checks for the presence of a hand inside the bounds, and if
found, return that hand along with activation and focus
information defined by the interactType.
</div>

|  |  |
|--|--|
|string id|An id for tracking element state. MUST be unique             within current hierarchy.|
|[Bounds]({{site.url}}/Pages/Reference/Bounds.html) bounds|Size and position of the volume, relative to             the current Hierarchy.|
|[UIConfirm]({{site.url}}/Pages/Reference/UIConfirm.html) interactType|UIConfirm.Pinch will activate when the             hand performs a 'pinch' gesture. UIConfirm.Push will activate              when the hand enters the volume, and behave the same as element's             focusState.|
|Handed& hand|This will be the last unpreoccupied hand found             inside the volume, and is the hand controlling the interaction.|
|BtnState& focusState|The focus state tells if the element has             a hand inside of the volume that qualifies for focus.|
|RETURNS: [BtnState]({{site.url}}/Pages/Reference/BtnState.html)|Based on the interactType, this is a BtnState that tells the activation state of the interaction.|

<div class='signature' markdown='1'>
```csharp
static BtnState VolumeAt(string id, Bounds bounds, UIConfirm interactType, Handed& hand)
```
A volume for helping to build one handed interactions.
This checks for the presence of a hand inside the bounds, and if
found, return that hand along with activation and focus
information defined by the interactType.
</div>

|  |  |
|--|--|
|string id|An id for tracking element state. MUST be unique             within current hierarchy.|
|[Bounds]({{site.url}}/Pages/Reference/Bounds.html) bounds|Size and position of the volume, relative to             the current Hierarchy.|
|[UIConfirm]({{site.url}}/Pages/Reference/UIConfirm.html) interactType|UIConfirm.Pinch will activate when the             hand performs a 'pinch' gesture. UIConfirm.Push will activate              when the hand enters the volume, and behave the same as element's             focusState.|
|Handed& hand|This will be the last unpreoccupied hand found             inside the volume, and is the hand controlling the interaction.|
|RETURNS: [BtnState]({{site.url}}/Pages/Reference/BtnState.html)|Based on the interactType, this is a BtnState that tells the activation state of the interaction.|

<div class='signature' markdown='1'>
```csharp
static BtnState VolumeAt(string id, Bounds bounds, UIConfirm interactType)
```
A volume for helping to build one handed interactions.
This checks for the presence of a hand inside the bounds, and if
found, return that hand along with activation and focus
information defined by the interactType.
</div>

|  |  |
|--|--|
|string id|An id for tracking element state. MUST be unique             within current hierarchy.|
|[Bounds]({{site.url}}/Pages/Reference/Bounds.html) bounds|Size and position of the volume, relative to             the current Hierarchy.|
|[UIConfirm]({{site.url}}/Pages/Reference/UIConfirm.html) interactType|UIConfirm.Pinch will activate when the             hand performs a 'pinch' gesture. UIConfirm.Push will activate              when the hand enters the volume, and behave the same as element's             focusState.|
|RETURNS: [BtnState]({{site.url}}/Pages/Reference/BtnState.html)|Based on the interactType, this is a BtnState that tells the activation state of the interaction.|





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

