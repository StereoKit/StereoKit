---
layout: default
title: Vec2.InRadius
description: Checks if a point is within a certain radius of this one. This is an easily readable shorthand of the squared distance check.
---
# [Vec2]({{site.url}}/Pages/Reference/Vec2.html).InRadius

<div class='signature' markdown='1'>
bool InRadius([Vec2]({{site.url}}/Pages/Reference/Vec2.html) pt, float radius)
</div>

|  |  |
|--|--|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) pt|The point to check against.|
|float radius|The distance to check against.|
|RETURNS: bool|True if the points are within radius of eachother, false not.|

Checks if a point is within a certain radius of this one.
This is an easily readable shorthand of the squared distance check.
<div class='signature' markdown='1'>
static bool InRadius([Vec2]({{site.url}}/Pages/Reference/Vec2.html) a, [Vec2]({{site.url}}/Pages/Reference/Vec2.html) b, float radius)
</div>

|  |  |
|--|--|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) a|The first point.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) b|And the second point!|
|float radius|The distance to check against.|
|RETURNS: bool|True if a and b are within radius of eachother, false not.|

Checks if two points are within a certain radius of
eachother. This is an easily readable shorthand of the squared
distance check.



