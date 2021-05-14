---
layout: default
title: SphericalHarmonics.Add
description: Adds a 'directional light' to the lighting approximation. This can be used to bake a multiple light setup, or accumulate light from a field of points.
---
# [SphericalHarmonics]({{site.url}}/Pages/Reference/SphericalHarmonics.html).Add

<div class='signature' markdown='1'>
void Add([Vec3]({{site.url}}/Pages/Reference/Vec3.html) lightDir, [Color]({{site.url}}/Pages/Reference/Color.html) lightColor)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) lightDir|Direction to the light source.|
|[Color]({{site.url}}/Pages/Reference/Color.html) lightColor|Color of the light, in linear color             space.|

Adds a 'directional light' to the lighting approximation.
This can be used to bake a multiple light setup, or accumulate
light from a field of points.



