---
layout: default
title: Ray.Intersect
description: Checks the intersection of this ray with a plane!
---
# [Ray]({{site.url}}/Pages/Reference/Ray.html).Intersect

<div class='signature' markdown='1'>
bool Intersect([Plane]({{site.url}}/Pages/Reference/Plane.html) plane, Vec3& at)
</div>

|  |  |
|--|--|
|[Plane]({{site.url}}/Pages/Reference/Plane.html) plane|Any plane you want to intersect with.|
|Vec3& at|An out parameter that will hold the intersection point.              If there's no intersection, this will be (0,0,0).|
|RETURNS: bool|True if there's an intersetion, false if not. Refer to the 'at' parameter for intersection information!|

Checks the intersection of this ray with a plane!
<div class='signature' markdown='1'>
bool Intersect([Sphere]({{site.url}}/Pages/Reference/Sphere.html) sphere, Vec3& at)
</div>

|  |  |
|--|--|
|[Sphere]({{site.url}}/Pages/Reference/Sphere.html) sphere|Any Sphere you want to intersect with.|
|Vec3& at|An out parameter that will hold the closest intersection              point to the ray's origin. If there's no intersection, this will be (0,0,0).|
|RETURNS: bool|True if intersection occurs, false if it doesn't. Refer to the 'at' parameter for intersection information!|

Checks the intersection of this ray with a sphere!
<div class='signature' markdown='1'>
bool Intersect([Bounds]({{site.url}}/Pages/Reference/Bounds.html) bounds, Vec3& at)
</div>

|  |  |
|--|--|
|[Bounds]({{site.url}}/Pages/Reference/Bounds.html) bounds|Any Bounds you want to intersect with.|
|Vec3& at|If the return is true, this point will be the closest intersection              point to the origin of the Ray. If there's no intersection, this will be (0,0,0).|
|RETURNS: bool|True if intersection occurs, false if it doesn't. Refer to the 'at' parameter for intersection information!|

Checks the intersection of this ray with a bounding box!



