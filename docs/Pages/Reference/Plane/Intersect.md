---
layout: default
title: Plane.Intersect
description: Checks the intersection of a ray with this plane!
---
# [Plane]({{site.url}}/Pages/Reference/Plane.html).Intersect

<div class='signature' markdown='1'>
bool Intersect([Ray]({{site.url}}/Pages/Reference/Ray.html) ray, Vec3& at)
</div>

|  |  |
|--|--|
|[Ray]({{site.url}}/Pages/Reference/Ray.html) ray|Ray we're checking with.|
|Vec3& at|An out parameter that will hold the intersection             point. If there's no intersection, this will be (0,0,0).|
|RETURNS: bool|True if there's an intersetion, false if not. Refer to the 'at' parameter for intersection information!|

Checks the intersection of a ray with this plane!
<div class='signature' markdown='1'>
bool Intersect([Vec3]({{site.url}}/Pages/Reference/Vec3.html) lineStart, [Vec3]({{site.url}}/Pages/Reference/Vec3.html) lineEnd, Vec3& at)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) lineStart|Start of the line.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) lineEnd|End of the line.|
|Vec3& at|An out parameter that will hold the intersection             point. If there's no intersection, this will be (0,0,0).|
|RETURNS: bool|True if there's an intersetion, false if not. Refer to the 'at' parameter for intersection information!|

Checks the intersection of a line with this plane!



