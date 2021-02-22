---
layout: default
title: FingerId
description: Index values for each finger! From 0-4, from thumb to little finger.
---
# FingerId

Index values for each finger! From 0-4, from thumb to little finger.




## Static Fields and Properties

|  |  |
|--|--|
|[FingerId]({{site.url}}/Pages/Reference/FingerId.html) [Index]({{site.url}}/Pages/Reference/FingerId/Index.html)|The primary index/pointer finger! Finger 1.|
|[FingerId]({{site.url}}/Pages/Reference/FingerId.html) [Little]({{site.url}}/Pages/Reference/FingerId/Little.html)|Finger 4, the smallest little finger! AKA, The Pinky.|
|[FingerId]({{site.url}}/Pages/Reference/FingerId.html) [Middle]({{site.url}}/Pages/Reference/FingerId/Middle.html)|Finger 2, next to the index finger.|
|[FingerId]({{site.url}}/Pages/Reference/FingerId.html) [Ring]({{site.url}}/Pages/Reference/FingerId/Ring.html)|Finger 3! What does one do with this finger? I guess... wear rings on it?|
|[FingerId]({{site.url}}/Pages/Reference/FingerId.html) [Thumb]({{site.url}}/Pages/Reference/FingerId/Thumb.html)|Finger 0.|



## Examples

Here's a small example of checking to see if a finger joint is inside
a box, and drawing an axis gizmo when it is!
```csharp
// A volume for checking inside of! 10cm on each side, at the origin
Bounds testArea = new Bounds(Vec3.One * 0.1f);

// This is a decent way to show we're working with both hands
for (int h = 0; h < (int)Handed.Max; h++)
{
	// Get the pose for the index fingertip
	Hand hand      = Input.Hand((Handed)h);
	Pose fingertip = hand[FingerId.Index, JointId.Tip].Pose;

	// Draw the fingertip pose axis if it's inside the volume
	if (testArea.Contains(fingertip.position))
		Lines.AddAxis(fingertip);
}
```

