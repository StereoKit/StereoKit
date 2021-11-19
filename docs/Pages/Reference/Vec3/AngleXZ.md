---
layout: default
title: Vec3.AngleXZ
description: Creates a vector that points out at the given 2D angle! This creates the vector on the XZ plane, and allows you to specify a constant y value.
---
# [Vec3]({{site.url}}/Pages/Reference/Vec3.html).AngleXZ

<div class='signature' markdown='1'>
```csharp
static Vec3 AngleXZ(float angleDeg, float y)
```
Creates a vector that points out at the given 2D angle!
This creates the vector on the XZ plane, and allows you to
specify a constant y value.
</div>

|  |  |
|--|--|
|float angleDeg|Angle in degrees, starting from (1,0) at             0, and continuing to (0,1) at 90.|
|float y|A constant value you can assign to the resulting             vector's y component.|
|RETURNS: [Vec3]({{site.url}}/Pages/Reference/Vec3.html)|A Vector pointing at the given angle! If y is zero, this will be a normalized vector (vector with a length of 1).|




