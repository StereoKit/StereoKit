---
layout: default
title: Handed
description: An enum for indicating which hand to use!
---
# Handed

An enum for indicating which hand to use!




## Static Fields and Properties

|  |  |
|--|--|
|[Handed]({{site.url}}/Pages/Reference/Handed.html) [Left]({{site.url}}/Pages/Reference/Handed/Left.html)|Left hand.|
|[Handed]({{site.url}}/Pages/Reference/Handed.html) [Max]({{site.url}}/Pages/Reference/Handed/Max.html)|The number of hands one generally has, this is much nicer than doing a for loop with '2' as the condition! It's much clearer when you can loop Hand.Max times instead.|
|[Handed]({{site.url}}/Pages/Reference/Handed.html) [Right]({{site.url}}/Pages/Reference/Handed/Right.html)|Right hand.|



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

