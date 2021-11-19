---
layout: default
title: Matrix.Decompose
description: Returns this transformation matrix to its original translation, rotation and scale components. Not exactly a cheap function. If this is not a transform matrix, there's a chance this call will fail, and return false.
---
# [Matrix]({{site.url}}/Pages/Reference/Matrix.html).Decompose

<div class='signature' markdown='1'>
```csharp
bool Decompose(Vec3& translation, Quat& rotation, Vec3& scale)
```
Returns this transformation matrix to its original
translation, rotation and scale components. Not exactly a cheap
function. If this is not a transform matrix, there's a chance
this call will fail, and return false.
</div>

|  |  |
|--|--|
|Vec3& translation|XYZ translation of the matrix.|
|Quat& rotation|The rotation quaternion, some lossiness             may be encountered when composing/decomposing.|
|Vec3& scale|XYZ scale components.|
|RETURNS: bool|If this is not a transform matrix, there's a chance this call will fail, and return false.|




