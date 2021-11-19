---
layout: default
title: Matrix.S
description: Creates a scaling Matrix, where scale can be different on each axis (non-uniform).
---
# [Matrix]({{site.url}}/Pages/Reference/Matrix.html).S

<div class='signature' markdown='1'>
```csharp
static Matrix S(Vec3 scale)
```
Creates a scaling Matrix, where scale can be different
on each axis (non-uniform).
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) scale|How much larger or smaller this transform             makes things. Vec3.One is a good default, as Vec3.Zero will             shrink it to nothing!|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|A non-uniform scaling matrix.|

<div class='signature' markdown='1'>
```csharp
static Matrix S(float scale)
```
Creates a scaling Matrix, where the scale is the same on
each axis (uniform).
</div>

|  |  |
|--|--|
|float scale|How much larger or smaller this transform             makes things. 1 is a good default, as 0 will shrink it to nothing!             This will expand to a scale vector of (size, size, size)|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|A uniform scaling matrix.|




