---
layout: default
title: Sphere.Intersect
description: Intersects a ray with this sphere, and finds if they intersect, and if so, where that intersection is! This only finds the closest intersection point to the origin of the ray.
---
# [Sphere]({{site.url}}/Pages/Reference/Sphere.html).Intersect

<div class='signature' markdown='1'>
```csharp
bool Intersect(Ray ray, Vec3& at)
```
Intersects a ray with this sphere, and finds if they intersect,
and if so, where that intersection is! This only finds the closest
intersection point to the origin of the ray.
</div>

|  |  |
|--|--|
|[Ray]({{site.url}}/Pages/Reference/Ray.html) ray|A ray to intersect with.|
|Vec3& at|An out parameter that will hold the closest intersection              point to the ray's origin. If there's not intersection, this will be (0,0,0).|
|RETURNS: bool|True if intersection occurs, false if it doesn't. Refer to the 'at' parameter for intersection information!|




