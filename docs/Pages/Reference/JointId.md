---
layout: default
title: JointId
description: Here's where hands get crazy! Technical terms, and watch out for the thumbs!
---
# enum JointId

Here's where hands get crazy! Technical terms, and watch out for
the thumbs!

## Enum Values

|  |  |
|--|--|
|KnuckleMajor|Joint 1. These are the knuckles at the top of the palm! For the thumb, Root and KnuckleMajor have the same value.|
|KnuckleMid|Joint 2. These are the knuckles in the middle of the finger! First joints on the fingers themselves.|
|KnuckleMinor|Joint 3. The joints right below the fingertip!|
|Root|Joint 0. This is at the base of the hand, right above the wrist. For the thumb, Root and KnuckleMajor have the same value.|
|Tip|Joint 4. The end/tip of each finger!|

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

