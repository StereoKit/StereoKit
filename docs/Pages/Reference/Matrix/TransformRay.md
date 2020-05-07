---
layout: default
title: Matrix.TransformRay
description: Shorthand to transform a ray though the Matrix! This properly transforms the position with the point transform method, and the direction with the direction transform method. Does not normalize, nor does it preserve a normalized direction if the Matrix contains scale data.
---
# [Matrix]({{site.url}}/Pages/Reference/Matrix.html).TransformRay

<div class='signature' markdown='1'>
[Ray]({{site.url}}/Pages/Reference/Ray.html) TransformRay([Ray]({{site.url}}/Pages/Reference/Ray.html) aRay)
</div>

|  |  |
|--|--|
|[Ray]({{site.url}}/Pages/Reference/Ray.html) aRay|A ray you wish to transform from one space to             another.|
|RETURNS: [Ray]({{site.url}}/Pages/Reference/Ray.html)|The transformed ray!|

Shorthand to transform a ray though the Matrix! This
properly transforms the position with the point transform method,
and the direction with the direction transform method. Does not
normalize, nor does it preserve a normalized direction if the
Matrix contains scale data.



