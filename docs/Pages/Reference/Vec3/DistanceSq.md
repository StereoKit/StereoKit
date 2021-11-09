---
layout: default
title: Vec3.DistanceSq
description: Calculates the distance between two points in space, but leaves them squared! Make sure they're in the same coordinate space! This is a fast function .)
---
# [Vec3]({{site.url}}/Pages/Reference/Vec3.html).DistanceSq

<div class='signature' markdown='1'>
```csharp
static float DistanceSq(Vec3 a, Vec3 b)
```
Calculates the distance between two points in space, but
leaves them squared! Make sure they're in the same coordinate
space! This is a fast function :)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) a|The first point.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) b|And the second point!|
|RETURNS: float|Distance between the two points, but squared!|





## Examples

```csharp
Vec3 pointA = new Vec3(3, 2, 5);
Vec3 pointB = new Vec3(3, 2, 8);

float distanceSquared = Vec3.DistanceSq(pointA, pointB);
if (distanceSquared < 4*4) { 
	Log.Info("Distance is less than 4");
}
```

