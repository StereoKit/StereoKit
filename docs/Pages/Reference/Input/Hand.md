---
layout: default
title: Input.Hand
description: Retrieves all the information about the user's hand! StereoKit will always provide hand information, however sometimes that information is simulated, like in the case of a mouse, or controllers.  Note that this is a copy of the hand information, and it's a good chunk of data, so it's a good idea to grab it once and keep it around for the frame, or at least function, rather than asking for it again and again each time you want to touch something.
---
# [Input]({{site.url}}/Pages/Reference/Input.html).Hand

<div class='signature' markdown='1'>
```csharp
static Hand Hand(Handed handed)
```
Retrieves all the information about the user's hand!
StereoKit will always provide hand information, however sometimes
that information is simulated, like in the case of a mouse, or
controllers.

Note that this is a copy of the hand information, and it's a good
chunk of data, so it's a good idea to grab it once and keep it
around for the frame, or at least function, rather than asking
for it again and again each time you want to touch something.
</div>

|  |  |
|--|--|
|[Handed]({{site.url}}/Pages/Reference/Handed.html) handed|Do you want the left or the right hand?|
|RETURNS: [Hand]({{site.url}}/Pages/Reference/Hand.html)|A copy of the entire set of hand data!|





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

