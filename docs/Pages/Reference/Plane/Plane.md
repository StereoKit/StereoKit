---
layout: default
title: Plane.Plane
description: Creates a Plane directly from the ax + by + cz + d = 0 formula!
---
# [Plane]({{site.url}}/Pages/Reference/Plane.html).Plane

<div class='signature' markdown='1'>
 Plane([Vec3]({{site.url}}/Pages/Reference/Vec3.html) normal, float d)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) normal|Direction the plane is facing.|
|float d|Distance along the normal from the origin to the surface of the plane.|

Creates a Plane directly from the ax + by + cz + d = 0
formula!
<div class='signature' markdown='1'>
 Plane([Vec3]({{site.url}}/Pages/Reference/Vec3.html) pointOnPlane, [Vec3]({{site.url}}/Pages/Reference/Vec3.html) planeNormal)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) pointOnPlane|Any point directly on the surface of the plane.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) planeNormal|Direction the plane is facing.|

Creates a plane from a normal, and any point on the plane!
<div class='signature' markdown='1'>
 Plane([Vec3]({{site.url}}/Pages/Reference/Vec3.html) pointOnPlane1, [Vec3]({{site.url}}/Pages/Reference/Vec3.html) pointOnPlane2, [Vec3]({{site.url}}/Pages/Reference/Vec3.html) pointOnPlane3)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) pointOnPlane1|First point on the plane.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) pointOnPlane2|Second point on the plane.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) pointOnPlane3|Third point on the plane.|

Creates a plane from 3 points that are directly on that plane.



