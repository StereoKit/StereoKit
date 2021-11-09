---
layout: default
title: Ray.At
description: Gets a point along the ray! This is basically just position + direction*percent. If Ray.direction is normalized, then percent is functionally distance, and can be used to find the point a certain distance out along the ray.
---
# [Ray]({{site.url}}/Pages/Reference/Ray.html).At

<div class='signature' markdown='1'>
```csharp
Vec3 At(float percent)
```
Gets a point along the ray! This is basically just
position + direction*percent. If Ray.direction is normalized,
then percent is functionally distance, and can be used to find
the point a certain distance out along the ray.
</div>

|  |  |
|--|--|
|float percent|How far along the ray should we get the              point at? This is in multiples of Ray.direction's magnitude. If             Ray.direction is normalized, this is functionally the distance.|
|RETURNS: [Vec3]({{site.url}}/Pages/Reference/Vec3.html)|The point at position + direction*percent.|




