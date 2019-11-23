---
layout: default
title: Bounds.Intersect
description: Calculate the intersection between a Ray, and these bounds. Returns false if no intersection occurred, and 'at' will contain the nearest intersection point to the start of the ray if an intersection is found!
---
# [Bounds]({{site.url}}/Pages/Reference/Bounds.html).Intersect

<div class='signature' markdown='1'>
bool Intersect([Ray]({{site.url}}/Pages/Reference/Ray.html) ray, Vec3& at)
</div>

|  |  |
|--|--|
|[Ray]({{site.url}}/Pages/Reference/Ray.html) ray|Any Ray in the same coordinate space as the Bounds.|
|Vec3& at|If the return is true, this point will be the closest intersection              point to the origin of the Ray.|
|RETURNS: bool|True if an intersection occurred, false if not.|

Calculate the intersection between a Ray, and these bounds. Returns false
if no intersection occurred, and 'at' will contain the nearest intersection point to
the start of the ray if an intersection is found!



