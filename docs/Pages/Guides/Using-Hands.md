---
layout: default
title: Using Hands
description: StereoKit uses a hands first approach to user input! Even when hand-sensors aren't available, hand data is simulated instead using existing devices! For ex...
---

# Using Hands

StereoKit uses a hands first approach to user input! Even when hand-sensors
aren't available, hand data is simulated instead using existing devices!
For example, Windows Mixed Reality controllers will blend between pre-recorded
hand poses based on button presses, as will mice. This way, fully articulated
hand data is always present for you to work with!

## Accessing Joints

![Hand with joints]({{site.url}}/img/screenshots/HandAxes.jpg)

Since hands are so central to interaction, accessing hand information needs
to be really easy to get! So here's how you might find the fingertip of the right
hand! If you ignore IsTracked, this'll give you the last known position for that
finger joint.
```csharp
Hand hand = Input.Hand(Handed.Right);
if (hand.IsTracked)
{ 
	Vec3 fingertip = hand[FingerId.Index, JointId.Tip].position;
}
```
Pretty straightforward! And if you prefer calling a function instead of using the
[] operator, that's cool too! You can call `hand.Get(FingerId.Index, JointId.Tip)`
instead!

If that's too granular for you, there's easy ways to check for pinching and
gripping! Pinched will tell you if a pinch is currently happening, JustPinched
will tell you if it just started being pinched this frame, and JustUnpinched will
tell you if the pinch just stopped this frame!
```csharp
if (hand.IsPinched) { }
if (hand.IsJustPinched) { }
if (hand.IsJustUnpinched) { }

if (hand.IsGripped) { }
if (hand.IsJustGripped) { }
if (hand.IsJustUngripped) { }
```
These are all convenience functions wrapping the `hand.pinchState` bit-flag, so you
can also use that directly if you want to do some bit-flag wizardry!
## Hand Menu

Lets imagine you want to make a hand menu, you might need to know
if the user is looking at the palm of their hand! Here's a quick
example of using the palm's pose and the dot product to determine
this.
```csharp
static bool HandFacingHead(Handed handed)
{
	Hand hand = Input.Hand(handed);
	if (!hand.IsTracked)
		return false;

	Vec3 palmDirection   = (hand.palm.Forward).Normalized;
	Vec3 directionToHead = (Input.Head.position - hand.palm.position).Normalized;

	return Vec3.Dot(palmDirection, directionToHead) > 0.5f;
}
```
Once you have that information, it's simply a matter of placing a
window off to the side of the hand! The palm pose Right direction
points to different sides of each hand, so a different X offset
is required for each hand.
```csharp
public static void DrawHandMenu(Handed handed)
{
	if (!HandFacingHead(handed))
		return;

	// Decide the size and offset of the menu
	Vec2  size   = new Vec2(4, 16);
	float offset = handed == Handed.Left ? -2-size.x : 2+size.x;

	// Position the menu relative to the side of the hand
	Hand hand   = Input.Hand(handed);
	Vec3 at     = hand[FingerId.Little, JointId.KnuckleMajor].position;
	Vec3 down   = hand[FingerId.Little, JointId.Root        ].position;
	Vec3 across = hand[FingerId.Index,  JointId.KnuckleMajor].position;

	Pose menuPose = new Pose(
		at,
		Quat.LookAt(at, across, at-down) * Quat.FromAngles(0, handed == Handed.Left ? 90 : -90, 0));
	menuPose.position += menuPose.Right * offset * U.cm;
	menuPose.position += menuPose.Up * (size.y/2) * U.cm;

	// And make a menu!
	UI.WindowBegin("HandMenu", ref menuPose, size * U.cm, UIWin.Empty);
	UI.Button("Test");
	UI.Button("That");
	UI.Button("Hand");
	UI.WindowEnd();
}
```
## Pointers

And lastly, StereoKit also has a pointer system! This applies to
more than just hands. Head, mouse, and other devices will also
create pointers into the scene. You can filter pointers based on
source family and device capabilities, so this is a great way to
abstract a few more input sources nicely!
```csharp
public static void DrawPointers()
{
	int hands = Input.PointerCount(InputSource.Hand);
	for (int i = 0; i < hands; i++)
	{
		Pointer pointer = Input.Pointer(i, InputSource.Hand);
		Lines.Add    (pointer.ray, 0.5f, Color.White, Units.mm2m);
		Lines.AddAxis(pointer.Pose);
	}
}
```
The code in context for this document can be found [on Github here](https://github.com/StereoKit/StereoKit/blob/master/Examples/StereoKitTest/DemoHands.cs)!

