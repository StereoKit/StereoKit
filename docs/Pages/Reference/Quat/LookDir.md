---
layout: default
title: Quat.LookDir
description: Creates a rotation from a resting direction, to the given direction! This prevents roll on the Z axis, Up is always (0,1,0)
---
# [Quat]({{site.url}}/Pages/Reference/Quat.html).LookDir

<div class='signature' markdown='1'>
```csharp
static Quat LookDir(Vec3 direction)
```
Creates a rotation from a resting direction, to the
given direction! This prevents roll on the Z axis, Up is always
(0,1,0)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) direction|Direction the rotation should be looking.             Doesn't need to be normalized.|
|RETURNS: [Quat]({{site.url}}/Pages/Reference/Quat.html)|A rotation from resting, to the given direction.|

<div class='signature' markdown='1'>
```csharp
static Quat LookDir(float x, float y, float z)
```
Creates a rotation from a resting direction, to the
given direction! This prevents roll on the Z axis, Up is always
(0,1,0)
</div>

|  |  |
|--|--|
|float x|X component of the direction the rotation should             be looking. Doesn't need to be normalized.|
|float y|Y component of the direction the rotation should             be looking. Doesn't need to be normalized.|
|float z|Z component of the direction the rotation should             be looking. Doesn't need to be normalized.|
|RETURNS: [Quat]({{site.url}}/Pages/Reference/Quat.html)|A rotation from resting, to the given direction.|




