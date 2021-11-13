---
layout: default
title: Matrix.Transform
description: Transforms a point through the Matrix! This is basically just multiplying a vector (x,y,z,1) with the Matrix.
---
# [Matrix]({{site.url}}/Pages/Reference/Matrix.html).Transform

<div class='signature' markdown='1'>
```csharp
Vec3 Transform(Vec3 point)
```
Transforms a point through the Matrix! This is basically
just multiplying a vector (x,y,z,1) with the Matrix.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) point|The point to transform.|
|RETURNS: [Vec3]({{site.url}}/Pages/Reference/Vec3.html)|The point transformed by the Matrix.|

<div class='signature' markdown='1'>
```csharp
Ray Transform(Ray ray)
```
Shorthand to transform a ray though the Matrix! This
properly transforms the position with the point transform method,
and the direction with the direction transform method. Does not
normalize, nor does it preserve a normalized direction if the
Matrix contains scale data.
</div>

|  |  |
|--|--|
|[Ray]({{site.url}}/Pages/Reference/Ray.html) ray|A ray you wish to transform from one space to             another.|
|RETURNS: [Ray]({{site.url}}/Pages/Reference/Ray.html)|The transformed ray!|

<div class='signature' markdown='1'>
```csharp
Pose Transform(Pose pose)
```
Shorthand for transforming a Pose! This will transform
the position of the Pose with the matrix, extract a rotation Quat
from the matrix and apply that to the Pose's orientation. Note
that extracting a rotation Quat is an expensive operation, so if
you're doing it more than once, you should cache the rotation
Quat and do this transform manually.
</div>

|  |  |
|--|--|
|[Pose]({{site.url}}/Pages/Reference/Pose.html) pose|The original pose.|
|RETURNS: [Pose]({{site.url}}/Pages/Reference/Pose.html)|The transformed pose.|




