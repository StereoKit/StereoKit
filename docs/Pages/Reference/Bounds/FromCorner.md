---
layout: default
title: Bounds.FromCorner
description: Create a bounding box from a corner, plus box dimensions.
---
# [Bounds]({{site.url}}/Pages/Reference/Bounds.html).FromCorner

<div class='signature' markdown='1'>
static [Bounds]({{site.url}}/Pages/Reference/Bounds.html) FromCorner([Vec3]({{site.url}}/Pages/Reference/Vec3.html) bottomLeftBack, [Vec3]({{site.url}}/Pages/Reference/Vec3.html) dimensions)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) bottomLeftBack|The -X,-Y,-Z corner of the box.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) dimensions|The total dimensions of the box.|
|RETURNS: [Bounds]({{site.url}}/Pages/Reference/Bounds.html)|A Bounds object that extends from bottomLeftBack to bottomLeftBack+dimensions.|

Create a bounding box from a corner, plus box dimensions.



