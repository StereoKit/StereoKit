---
layout: default
title: Hierarchy.ToLocalDirection
description: Converts a world space direction into the local space of the current Hierarchy stack! This excludes the translation component normally applied to vectors, so it's still a valid direction.
---
# [Hierarchy]({{site.url}}/Pages/Reference/Hierarchy.html).ToLocalDirection

<div class='signature' markdown='1'>
static [Vec3]({{site.url}}/Pages/Reference/Vec3.html) ToLocalDirection([Vec3]({{site.url}}/Pages/Reference/Vec3.html) worldDirection)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) worldDirection|A direction in world space.|
|RETURNS: [Vec3]({{site.url}}/Pages/Reference/Vec3.html)|The provided direction now in local hierarchy space!|

Converts a world space direction into the local space of
the current Hierarchy stack! This excludes the translation
component normally applied to vectors, so it's still a valid
direction.



