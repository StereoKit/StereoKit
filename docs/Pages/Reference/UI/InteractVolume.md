---
layout: default
title: UI.InteractVolume
description: This watches a volume of space for pinch interaction events! If a hand is inside the space indicated by the bounds, this function will return that hand's pinch state, as well as indicate which hand did it through the out parameter.  Note that since this only provides the hand's pinch state, it won't give you JustActive and JustInactive notifications for when the hand enters or leaves the volume.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).InteractVolume

<div class='signature' markdown='1'>
```csharp
static BtnState InteractVolume(Bounds bounds, Handed& hand)
```
This watches a volume of space for pinch interaction
events! If a hand is inside the space indicated by the bounds,
this function will return that hand's pinch state, as well as
indicate which hand did it through the out parameter.

Note that since this only provides the hand's pinch state, it
won't give you JustActive and JustInactive notifications for
when the hand enters or leaves the volume.
</div>

|  |  |
|--|--|
|[Bounds]({{site.url}}/Pages/Reference/Bounds.html) bounds|A UI hierarchy space bounding volume.|
|Handed& hand|This will be the last hand that provides a              pinch state within this volume. That means that if both hands are             pinching in this volume, it will provide the Right hand.|
|RETURNS: [BtnState]({{site.url}}/Pages/Reference/BtnState.html)|This will be the pinch state of the last hand that provides a pinch state within this volume. That means that if both hands are pinching in this volume, it will provide the pinch state of the Right hand.|




