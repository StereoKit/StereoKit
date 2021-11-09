---
layout: default
title: Bounds.Contains
description: Does the Bounds contain the given point? This includes points that are on the surface of the Bounds.
---
# [Bounds]({{site.url}}/Pages/Reference/Bounds.html).Contains

<div class='signature' markdown='1'>
```csharp
bool Contains(Vec3 pt)
```
Does the Bounds contain the given point? This includes points that are on
the surface of the Bounds.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) pt|A point in the same coordinate space as the Bounds.|
|RETURNS: bool|True if the point is on, or in the Bounds.|

<div class='signature' markdown='1'>
```csharp
bool Contains(Vec3 linePt1, Vec3 linePt2)
```
Does the Bounds contain or intersects with the given line?
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) linePt1|Start of the line|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) linePt2|End of the line|
|RETURNS: bool|True if the line is in, or intersects with the bounds.|

<div class='signature' markdown='1'>
```csharp
bool Contains(Vec3 linePt1, Vec3 linePt2, float radius)
```
Does the bounds contain or intersect with the given
capsule?
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) linePt1|Start of the capsule.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) linePt2|End of the capsule|
|float radius|Radius of the capsule.|
|RETURNS: bool|True if the capsule is in, or intersects with the bounds.|





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

