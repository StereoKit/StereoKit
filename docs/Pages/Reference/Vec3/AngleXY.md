---
layout: default
title: Vec3.AngleXY
description: Creates a vector that points out at the given 2D angle! This creates the vector on the XY plane, and allows you to specify a constant z value.
---
# [Vec3]({{site.url}}/Pages/Reference/Vec3.html).AngleXY

<div class='signature' markdown='1'>
```csharp
static Vec3 AngleXY(float angleDeg, float z)
```
Creates a vector that points out at the given 2D angle!
This creates the vector on the XY plane, and allows you to
specify a constant z value.
</div>

|  |  |
|--|--|
|float angleDeg|Angle in degrees, starting from (1,0) at             0, and continuing to (0,1) at 90.|
|float z|A constant value you can assign to the resulting             vector's z component.|
|RETURNS: [Vec3]({{site.url}}/Pages/Reference/Vec3.html)|A vector pointing at the given angle! If z is zero, this will be a normalized vector (vector with a length of 1).|




