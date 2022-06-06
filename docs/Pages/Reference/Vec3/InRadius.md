---
layout: default
title: Vec3.InRadius
description: Checks if a point is within a certain radius of this one. This is an easily readable shorthand of the squared distance check.
---
# [Vec3]({{site.url}}/Pages/Reference/Vec3.html).InRadius

<div class='signature' markdown='1'>
```csharp
bool InRadius(Vec3 pt, float radius)
```
Checks if a point is within a certain radius of this one.
This is an easily readable shorthand of the squared distance check.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) pt|The point to check against.|
|float radius|The distance to check against.|
|RETURNS: bool|True if the points are within radius of each other, false not.|

<div class='signature' markdown='1'>
```csharp
static bool InRadius(Vec3 a, Vec3 b, float radius)
```
Checks if two points are within a certain radius of
each other. This is an easily readable shorthand of the squared
distance check.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) a|The first point.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) b|And the second point!|
|float radius|The distance to check against.|
|RETURNS: bool|True if a and b are within radius of each other, false if not.|




