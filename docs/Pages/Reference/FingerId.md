---
layout: default
title: FingerId
description: Index values for each finger! From 0-4, from thumb to little finger.
---
# enum FingerId

Index values for each finger! From 0-4, from thumb to little finger.

## Enum Values

|  |  |
|--|--|
|Index|The primary index/pointer finger! Finger 1.|
|Little|Finger 4, the smallest little finger! AKA, The Pinky.|
|Middle|Finger 2, next to the index finger.|
|Ring|Finger 3! What does one do with this finger? I guess... wear rings on it?|
|Thumb|Finger 0.|

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

