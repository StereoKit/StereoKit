---
layout: default
title: Plane.Plane
description: Creates a Plane directly from the ax + by + cz + d = 0 formula!
---
# [Plane]({{site.url}}/Pages/Reference/Plane.html).Plane

<div class='signature' markdown='1'>
```csharp
void Plane(Vec3 normal, float d)
```
Creates a Plane directly from the ax + by + cz + d = 0
formula!
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) normal|Direction the plane is facing.|
|float d|Distance along the normal from the origin to the surface of the plane.|

<div class='signature' markdown='1'>
```csharp
void Plane(Vec3 pointOnPlane, Vec3 planeNormal)
```
Creates a plane from a normal, and any point on the plane!
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) pointOnPlane|Any point directly on the surface of the plane.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) planeNormal|Direction the plane is facing.|

<div class='signature' markdown='1'>
```csharp
void Plane(Vec3 pointOnPlane1, Vec3 pointOnPlane2, Vec3 pointOnPlane3)
```
Creates a plane from 3 points that are directly on that plane.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) pointOnPlane1|First point on the plane.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) pointOnPlane2|Second point on the plane.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) pointOnPlane3|Third point on the plane.|




