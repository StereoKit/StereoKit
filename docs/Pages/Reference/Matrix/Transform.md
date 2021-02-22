---
layout: default
title: Matrix.Transform
description: Transforms a point through the Matrix! This is basically just multiplying a vector (x,y,z,1) with the Matrix.
---
# [Matrix]({{site.url}}/Pages/Reference/Matrix.html).Transform

<div class='signature' markdown='1'>
[Vec3]({{site.url}}/Pages/Reference/Vec3.html) Transform([Vec3]({{site.url}}/Pages/Reference/Vec3.html) point)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) point|The point to transform.|
|RETURNS: [Vec3]({{site.url}}/Pages/Reference/Vec3.html)|The point transformed by the Matrix.|

Transforms a point through the Matrix! This is basically
just multiplying a vector (x,y,z,1) with the Matrix.
<div class='signature' markdown='1'>
[Ray]({{site.url}}/Pages/Reference/Ray.html) Transform([Ray]({{site.url}}/Pages/Reference/Ray.html) ray)
</div>

|  |  |
|--|--|
|[Ray]({{site.url}}/Pages/Reference/Ray.html) ray|A ray you wish to transform from one space to             another.|
|RETURNS: [Ray]({{site.url}}/Pages/Reference/Ray.html)|The transformed ray!|

Shorthand to transform a ray though the Matrix! This
properly transforms the position with the point transform method,
and the direction with the direction transform method. Does not
normalize, nor does it preserve a normalized direction if the
Matrix contains scale data.



