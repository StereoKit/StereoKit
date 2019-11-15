---
layout: default
title: Bounds.Contains
description: Does the Bounds contain the given point? This includes points that are on the surface of the Bounds.
---
# [Bounds]({{site.url}}/Pages/Reference/Bounds.html).Contains

<div class='signature' markdown='1'>
bool Contains([Vec3]({{site.url}}/Pages/Reference/Vec3.html) pt)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) pt|A point in the same coordinate space as the Bounds.|
|RETURNS: bool|True if the point is on, or in the Bounds.|

Does the Bounds contain the given point? This includes points that are on
the surface of the Bounds.
<div class='signature' markdown='1'>
bool Contains([Vec3]({{site.url}}/Pages/Reference/Vec3.html) linePt1, [Vec3]({{site.url}}/Pages/Reference/Vec3.html) linePt2)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) linePt1|Start of the line|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) linePt2|End of the line|
|RETURNS: bool|True if the line is in, or intersects with the bounds.|

Does the Bounds contain or intersects with the given line?



