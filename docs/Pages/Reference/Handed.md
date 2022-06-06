---
layout: default
title: Handed
description: An enum for indicating which hand to use!
---
# enum Handed

An enum for indicating which hand to use!

## Enum Values

|  |  |
|--|--|
|Left|Left hand.|
|Max|The number of hands one generally has, this is much nicer than doing a for loop with '2' as the condition! It's much clearer when you can loop Hand.Max times instead.|
|Right|Right hand.|

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

