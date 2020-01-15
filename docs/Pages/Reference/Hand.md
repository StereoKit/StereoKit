---
layout: default
title: Hand
description: Information about a hand!
---
# Hand

Information about a hand!


## Instance Fields and Properties

|  |  |
|--|--|
|[BtnState]({{site.url}}/Pages/Reference/BtnState.html) [grip]({{site.url}}/Pages/Reference/Hand/grip.html)|Is the hand making a grip gesture right now? Fingers next to the palm.|
|[Handed]({{site.url}}/Pages/Reference/Handed.html) [handed]({{site.url}}/Pages/Reference/Hand/handed.html)|Is this a right hand, or a left hand?|
|[Pose]({{site.url}}/Pages/Reference/Pose.html) [palm]({{site.url}}/Pages/Reference/Hand/palm.html)|The position and orientation at the center of the palm! Here, Forward is the direction the palm is facing. X+ is to the outside of the right hand, and to the inside of the left hand.|
|[BtnState]({{site.url}}/Pages/Reference/BtnState.html) [pinch]({{site.url}}/Pages/Reference/Hand/pinch.html)|Is the hand making a pinch gesture right now? Finger and thumb together.|
|[BtnState]({{site.url}}/Pages/Reference/BtnState.html) [tracked]({{site.url}}/Pages/Reference/Hand/tracked.html)|Is the hand being tracked by the sensors right now?|
|[Pose]({{site.url}}/Pages/Reference/Pose.html) [wrist]({{site.url}}/Pages/Reference/Hand/wrist.html)|Pose of the wrist. TODO: Not populated right now.|



## Static Fields and Properties

|  |  |
|--|--|
|bool [IsGripped]({{site.url}}/Pages/Reference/Hand/IsGripped.html)|Are the fingers currently gripped?|
|bool [IsJustGripped]({{site.url}}/Pages/Reference/Hand/IsJustGripped.html)|Have the fingers just been gripped this frame?|
|bool [IsJustPinched]({{site.url}}/Pages/Reference/Hand/IsJustPinched.html)|Have the fingers just been pinched this frame?|
|bool [IsJustTracked]({{site.url}}/Pages/Reference/Hand/IsJustTracked.html)|Has the hand just started being tracked this frame?|
|bool [IsJustUngripped]({{site.url}}/Pages/Reference/Hand/IsJustUngripped.html)|Have the fingers just stopped being gripped this frame?|
|bool [IsJustUnpinched]({{site.url}}/Pages/Reference/Hand/IsJustUnpinched.html)|Have the fingers just stopped being pinched this frame?|
|bool [IsJustUntracked]({{site.url}}/Pages/Reference/Hand/IsJustUntracked.html)|Has the hand just stopped being tracked this frame?|
|bool [IsPinched]({{site.url}}/Pages/Reference/Hand/IsPinched.html)|Are the fingers currently pinched?|
|bool [IsTracked]({{site.url}}/Pages/Reference/Hand/IsTracked.html)|Is the hand being tracked by the sensors right now?|
|[Material]({{site.url}}/Pages/Reference/Material.html) [Material]({{site.url}}/Pages/Reference/Hand/Material.html)|Set the Material used to render the hand! The default material uses an offset of -10 for optimization and transparency order.|
|bool [Solid]({{site.url}}/Pages/Reference/Hand/Solid.html)|Should the hand be considered solid by the physics system?|
|bool [Visible]({{site.url}}/Pages/Reference/Hand/Visible.html)|Should the hand be rendered?|



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

