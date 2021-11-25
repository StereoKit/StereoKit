---
layout: default
title: Quat.LookDir
description: Creates a rotation that describes looking towards a direction. This is great for quickly describing facing behavior! This rotation works best when applied to objects that face Vec3.Forward in their resting/model space pose.
---
# [Quat]({{site.url}}/Pages/Reference/Quat.html).LookDir

<div class='signature' markdown='1'>
```csharp
static Quat LookDir(Vec3 direction)
```
Creates a rotation that describes looking towards a
direction. This is great for quickly describing facing behavior!
This rotation works best when applied to objects that face
Vec3.Forward in their resting/model space pose.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) direction|Direction the rotation should be looking.             Doesn't need to be normalized.|
|RETURNS: [Quat]({{site.url}}/Pages/Reference/Quat.html)|A rotation that describes looking towards a direction.|

<div class='signature' markdown='1'>
```csharp
static Quat LookDir(float x, float y, float z)
```
Creates a rotation that describes looking towards a
direction. This is great for quickly describing facing behavior!
This rotation works best when applied to objects that face
Vec3.Forward in their resting/model space pose.
</div>

|  |  |
|--|--|
|float x|X component of the direction the rotation should             be looking. Doesn't need to be normalized.|
|float y|Y component of the direction the rotation should             be looking. Doesn't need to be normalized.|
|float z|Z component of the direction the rotation should             be looking. Doesn't need to be normalized.|
|RETURNS: [Quat]({{site.url}}/Pages/Reference/Quat.html)|A rotation that describes looking towards a direction.|




