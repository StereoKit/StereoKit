---
layout: default
title: Matrix.*
description: Multiplies two matrices together! This is a great way to combine transform operations. Note that StereoKit's matrices are row-major, and multiplication order is important! To translate, then scale, multiple in order of 'translate * scale'.
---
# [Matrix]({{site.url}}/Pages/Reference/Matrix.html).*

<div class='signature' markdown='1'>
```csharp
static Matrix *(Matrix a, Matrix b)
```
Multiplies two matrices together! This is a great way to
combine transform operations. Note that StereoKit's matrices are
row-major, and multiplication order is important! To translate,
then scale, multiple in order of 'translate * scale'.
</div>

|  |  |
|--|--|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) a|First Matrix.|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) b|Second Matrix.|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|Result of matrix multiplication.|

<div class='signature' markdown='1'>
```csharp
static Vec3 *(Matrix a, Vec3 b)
```
Multiplies the vector by the Matrix! Since only a 1x4
vector can be multiplied against a 4x4 matrix, this uses '1' for
the 4th element, so the result will also include translation! To
exclude translation, use `Matrix.TransformNormal`.
</div>

|  |  |
|--|--|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) a|A transform matrix.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) b|Any Vector.|
|RETURNS: [Vec3]({{site.url}}/Pages/Reference/Vec3.html)|The Vec3 transformed by the matrix, including translation.|

<div class='signature' markdown='1'>
```csharp
static Ray *(Matrix a, Ray b)
```
Transforms a Ray by the Matrix! The position and direction
are both multiplied by the matrix, accounting properly for which
should include translation, and which should not.
</div>

|  |  |
|--|--|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) a|A transform matrix.|
|[Ray]({{site.url}}/Pages/Reference/Ray.html) b|A Ray to be transformed.|
|RETURNS: [Ray]({{site.url}}/Pages/Reference/Ray.html)|A Ray transformed by the Matrix.|

<div class='signature' markdown='1'>
```csharp
static Pose *(Matrix a, Pose b)
```
Transforms a Pose by the Matrix! The position and
orientation are both transformed by the matrix, accounting properly
for the Pose's quaternion.
</div>

|  |  |
|--|--|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) a|A transform matrix.|
|[Pose]({{site.url}}/Pages/Reference/Pose.html) b|A Pose to be transformed.|
|RETURNS: [Pose]({{site.url}}/Pages/Reference/Pose.html)|A Ray transformed by the Matrix.|




