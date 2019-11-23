---
layout: default
title: Pose.ToMatrix
description: Converts this pose into a transform matrix, incorporating a provided scale value.
---
# [Pose]({{site.url}}/Pages/Reference/Pose.html).ToMatrix

<div class='signature' markdown='1'>
[Matrix]({{site.url}}/Pages/Reference/Matrix.html) ToMatrix([Vec3]({{site.url}}/Pages/Reference/Vec3.html) scale)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) scale|A scale vector! Vec3.One would be an identity scale.|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|A Matrix that transforms to the given pose.|

Converts this pose into a transform matrix, incorporating a provided scale value.
<div class='signature' markdown='1'>
[Matrix]({{site.url}}/Pages/Reference/Matrix.html) ToMatrix(float scale)
</div>

|  |  |
|--|--|
|float scale|A uniform scale factor! 1 would be an identity scale.|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|A Matrix that transforms to the given pose.|

Converts this pose into a transform matrix, incorporating a provided scale value.
<div class='signature' markdown='1'>
[Matrix]({{site.url}}/Pages/Reference/Matrix.html) ToMatrix()
</div>

|  |  |
|--|--|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|A Matrix that transforms to the given pose.|

Converts this pose into a transform matrix.



