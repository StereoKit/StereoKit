---
layout: default
title: Pose.ToMatrix
description: Converts this pose into a transform matrix, incorporating a provided scale value.
---
# [Pose]({{site.url}}/Pages/Reference/Pose.html).ToMatrix

<div class='signature' markdown='1'>
```csharp
Matrix ToMatrix(Vec3 scale)
```
Converts this pose into a transform matrix, incorporating a provided scale value.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) scale|A scale vector! Vec3.One would be an identity scale.|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|A Matrix that transforms to the given pose.|

<div class='signature' markdown='1'>
```csharp
Matrix ToMatrix(float scale)
```
Converts this pose into a transform matrix, incorporating a provided scale value.
</div>

|  |  |
|--|--|
|float scale|A uniform scale factor! 1 would be an identity scale.|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|A Matrix that transforms to the given pose.|

<div class='signature' markdown='1'>
```csharp
Matrix ToMatrix()
```
Converts this pose into a transform matrix.
</div>

|  |  |
|--|--|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|A Matrix that transforms to the given pose.|




