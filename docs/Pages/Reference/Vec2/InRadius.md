---
layout: default
title: Vec2.InRadius
description: Checks if a point is within a certain radius of this one. This is an easily readable shorthand of the squared distance check.
---
# [Vec2]({{site.url}}/Pages/Reference/Vec2.html).InRadius

<div class='signature' markdown='1'>
```csharp
bool InRadius(Vec2 pt, float radius)
```
Checks if a point is within a certain radius of this one.
This is an easily readable shorthand of the squared distance check.
</div>

|  |  |
|--|--|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) pt|The point to check against.|
|float radius|The distance to check against.|
|RETURNS: bool|True if the points are within radius of each other, false not.|

<div class='signature' markdown='1'>
```csharp
static bool InRadius(Vec2 a, Vec2 b, float radius)
```
Checks if two points are within a certain radius of
each other. This is an easily readable shorthand of the squared
distance check.
</div>

|  |  |
|--|--|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) a|The first point.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) b|And the second point!|
|float radius|The distance to check against.|
|RETURNS: bool|True if a and b are within radius of each other, false if not.|




