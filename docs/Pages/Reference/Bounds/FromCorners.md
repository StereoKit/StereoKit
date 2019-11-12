---
layout: default
title: Bounds.FromCorners
description: Create a bounding box between two corner points.
---
# [Bounds]({{site.url}}/Pages/Reference/Bounds.html).FromCorners

<div class='signature' markdown='1'>
static [Bounds]({{site.url}}/Pages/Reference/Bounds.html) FromCorners([Vec3]({{site.url}}/Pages/Reference/Vec3.html) bottomLeftBack, [Vec3]({{site.url}}/Pages/Reference/Vec3.html) topRightFront)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) bottomLeftBack|The -X,-Y,-Z corner of the box.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) topRightFront|The +X,+Y,+Z corner of the box.|
|RETURNS: [Bounds]({{site.url}}/Pages/Reference/Bounds.html)|A Bounds object that extends from bottomLeftBack to topRightFront.|

Create a bounding box between two corner points.



