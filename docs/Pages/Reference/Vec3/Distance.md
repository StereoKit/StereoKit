---
layout: default
title: Vec3.Distance
description: Calculates the distance between two points in space! Make sure they're in the same coordinate space! Uses a Sqrt, so it's not blazing fast, prefer DistanceSq when possible.
---
# [Vec3]({{site.url}}/Pages/Reference/Vec3.html).Distance

<div class='signature' markdown='1'>
static float Distance([Vec3]({{site.url}}/Pages/Reference/Vec3.html) a, [Vec3]({{site.url}}/Pages/Reference/Vec3.html) b)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) a|The first point.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) b|And the second point!|
|RETURNS: float|Distance between the two points.|

Calculates the distance between two points in space!
Make sure they're in the same coordinate space! Uses a Sqrt, so
it's not blazing fast, prefer DistanceSq when possible.




## Examples

```csharp
Vec3  pointA   = new Vec3(3,2,5);
Vec3  pointB   = new Vec3(3,2,8);
float distance = Vec3.Distance(pointA, pointB);
```

