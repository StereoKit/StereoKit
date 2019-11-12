---
layout: default
title: Matrix.TransformDirection
description: Transforms a point through the Matrix, but excluding translation! This is great for transforming vectors that are -directions- rather than points in space. Use this to transform normals and directions. The same as multiplying (x,y,z,0) with the Matrix.
---
# [Matrix]({{site.url}}/Pages/Reference/Matrix.html).TransformDirection

<div class='signature' markdown='1'>
[Vec3]({{site.url}}/Pages/Reference/Vec3.html) TransformDirection([Vec3]({{site.url}}/Pages/Reference/Vec3.html) direction)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) direction|The direction to transform.|
|RETURNS: [Vec3]({{site.url}}/Pages/Reference/Vec3.html)|The direction transformed by the Matrix.|

Transforms a point through the Matrix, but excluding translation! This is great
for transforming vectors that are -directions- rather than points in space. Use this to
transform normals and directions. The same as multiplying (x,y,z,0) with the Matrix.



