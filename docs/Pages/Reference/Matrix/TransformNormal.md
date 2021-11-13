---
layout: default
title: Matrix.TransformNormal
description: Transforms a point through the Matrix, but excluding translation! This is great for transforming vectors that are -directions- rather than points in space. Use this to transform normals and directions. The same as multiplying (x,y,z,0) with the Matrix.
---
# [Matrix]({{site.url}}/Pages/Reference/Matrix.html).TransformNormal

<div class='signature' markdown='1'>
```csharp
Vec3 TransformNormal(Vec3 normal)
```
Transforms a point through the Matrix, but excluding
translation! This is great for transforming vectors that are
-directions- rather than points in space. Use this to transform
normals and directions. The same as multiplying (x,y,z,0) with
the Matrix.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) normal|The direction to transform.|
|RETURNS: [Vec3]({{site.url}}/Pages/Reference/Vec3.html)|The direction transformed by the Matrix.|




