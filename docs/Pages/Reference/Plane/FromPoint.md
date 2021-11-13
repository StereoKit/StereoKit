---
layout: default
title: Plane.FromPoint
description: Creates a plane from a normal, and any point on the plane!
---
# [Plane]({{site.url}}/Pages/Reference/Plane.html).FromPoint

<div class='signature' markdown='1'>
```csharp
static Plane FromPoint(Vec3 pointOnPlane, Vec3 planeNormal)
```
Creates a plane from a normal, and any point on the
plane!
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) pointOnPlane|Any point directly on the surface of              the plane.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) planeNormal|Direction the plane is facing.|
|RETURNS: [Plane]({{site.url}}/Pages/Reference/Plane.html)|A plane that contains pointOnPlane, and faces planeNormal.|




