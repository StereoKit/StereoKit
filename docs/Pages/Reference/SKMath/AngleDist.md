---
layout: default
title: SKMath.AngleDist
description: Calculates the minimum angle 'distance' between two angles. This covers wraparound cases like. the minimum distance between 10 and 350 is 20. Only works on angles in the range of 0-360.
---
# [SKMath]({{site.url}}/Pages/Reference/SKMath.html).AngleDist

<div class='signature' markdown='1'>
```csharp
static float AngleDist(float a, float b)
```
Calculates the minimum angle 'distance' between two
angles. This covers wraparound cases like: the minimum distance
between 10 and 350 is 20. Only works on angles in the range of
0-360.
</div>

|  |  |
|--|--|
|float a|First angle, in degrees.|
|float b|Second angle, in degrees.|
|RETURNS: float|Degrees 0-180, the minimum angle between a and b.|




