---
layout: default
title: Quat.LookAt
description: Creates a rotation that describes looking from a point, to another point! This is a great function for camera style rotation, or other facing behavior when you know where an object is, and where you want it to look at. This rotation works best when applied to objects that face Vec3.Forward in their resting/model space pose.
---
# [Quat]({{site.url}}/Pages/Reference/Quat.html).LookAt

<div class='signature' markdown='1'>
```csharp
static Quat LookAt(Vec3 lookFromPoint, Vec3 lookAtPoint, Vec3 upDirection)
```
Creates a rotation that describes looking from a point,
to another point! This is a great function for camera style
rotation, or other facing behavior when you know where an object
is, and where you want it to look at. This rotation works best
when applied to objects that face Vec3.Forward in their
resting/model space pose.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) lookFromPoint|Position of where the 'object' is.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) lookAtPoint|Position of where the object should be             looking!|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) upDirection|A direction indicating where 'up' is             from this rotation's persepective. A 'normal' up direction would             be (0,1,0).|
|RETURNS: [Quat]({{site.url}}/Pages/Reference/Quat.html)|A rotation that describes looking from a point, to another point.|

<div class='signature' markdown='1'>
```csharp
static Quat LookAt(Vec3 lookFromPoint, Vec3 lookAtPoint)
```
Creates a rotation that describes looking from a point,
to another point! This is a great function for camera style
rotation, or other facing behavior when you know where an object
is, and where you want it to look at. This rotation works best
when applied to objects that face Vec3.Forward in their
resting/model space pose.

This overload automatically defines 'up' as (0,1,0), which is the
most common way of using this type of rotation.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) lookFromPoint|Position of where the 'object' is.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) lookAtPoint|Position of where the object should be             looking!|
|RETURNS: [Quat]({{site.url}}/Pages/Reference/Quat.html)|A rotation that describes looking from a point, to another point.|





## Examples

Quat.LookAt and LookDir are probably one of the easiest ways to
work with quaternions in StereoKit! They're handy functions to
have a good understanding of. Here's an example of how you might
use them.
```csharp
// Draw a box that always rotates to face the user
Vec3 boxPos = new Vec3(1,0,1);
Quat boxRot = Quat.LookAt(boxPos, Input.Head.position);
Mesh.Cube.Draw(Material.Default, Matrix.TR(boxPos, boxRot));

// Make a Window that faces a user that enters the app looking
// Forward.
Pose winPose = new Pose(0,0,-0.5f, Quat.LookDir(0,0,1));
UI.WindowBegin("Posed Window", ref winPose);
UI.WindowEnd();

```

