---
layout: default
title: Hierarchy.ToLocal
description: Converts a world space point into the local space of the current Hierarchy stack!
---
# [Hierarchy]({{site.url}}/Pages/Reference/Hierarchy.html).ToLocal

<div class='signature' markdown='1'>
static [Vec3]({{site.url}}/Pages/Reference/Vec3.html) ToLocal([Vec3]({{site.url}}/Pages/Reference/Vec3.html) worldPoint)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) worldPoint|A point in world space.|
|RETURNS: [Vec3]({{site.url}}/Pages/Reference/Vec3.html)|The provided point now in local hierarchy space!|

Converts a world space point into the local space of the
current Hierarchy stack!
<div class='signature' markdown='1'>
static [Quat]({{site.url}}/Pages/Reference/Quat.html) ToLocal([Quat]({{site.url}}/Pages/Reference/Quat.html) worldOrientation)
</div>

|  |  |
|--|--|
|[Quat]({{site.url}}/Pages/Reference/Quat.html) worldOrientation|A rotation in world space.|
|RETURNS: [Quat]({{site.url}}/Pages/Reference/Quat.html)|The provided rotation now in local hierarchy space!|

Converts a world space rotation into the local space of
the current Hierarchy stack!



