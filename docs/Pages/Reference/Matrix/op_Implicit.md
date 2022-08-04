---
layout: default
title: Matrix.Implicit Conversions
description: Allows implicit conversion from System.Numerics.Matrix4x4 to StereoKit.Matrix.
---
# [Matrix]({{site.url}}/Pages/Reference/Matrix.html).Implicit Conversions

<div class='signature' markdown='1'>
```csharp
static Matrix Implicit Conversions(Matrix4x4 m)
```
Allows implicit conversion from System.Numerics.Matrix4x4
to StereoKit.Matrix.
</div>

|  |  |
|--|--|
|Matrix4x4 m|Any System.Numerics Matrix4x4.|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|A StereoKit compatible matrix.|

<div class='signature' markdown='1'>
```csharp
static Matrix4x4 Implicit Conversions(Matrix m)
```
Allows implicit conversion from StereoKit.Matrix to
System.Numerics.Matrix4x4
</div>

|  |  |
|--|--|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) m|Any StereoKit.Matrix.|
|RETURNS: Matrix4x4|A System.Numerics compatible matrix.|




