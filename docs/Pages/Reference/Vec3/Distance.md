---
layout: default
title: Vec3.Distance
description: Calculates the distance between two points in space! Make sure they're in the same coordinate space! Uses a Sqrt, so it's not blazing fast, prefer DistanceSq when possible.
---
# [Vec3]({{site.url}}/Pages/Reference/Vec3.html).Distance

<div class='signature' markdown='1'>
```csharp
static float Distance(Vec3 a, Vec3 b)
```
Calculates the distance between two points in space!
Make sure they're in the same coordinate space! Uses a Sqrt, so
it's not blazing fast, prefer DistanceSq when possible.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) a|The first point.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) b|And the second point!|
|RETURNS: float|Distance between the two points.|





## Examples

### Distance between two points

Distance does use a Sqrt call, so only use it if you definitely
need the actual distance. Otherwise, consider DistanceSq.
```csharp
Vec3  pointA   = new Vec3(3,2,5);
Vec3  pointB   = new Vec3(3,2,8);
float distance = Vec3.Distance(pointA, pointB);
```

