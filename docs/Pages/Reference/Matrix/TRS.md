---
layout: default
title: Matrix.TRS
description: Translate, Rotate, Scale. Creates a transform Matrix using all these components!
---
# [Matrix]({{site.url}}/Pages/Reference/Matrix.html).TRS

<div class='signature' markdown='1'>
```csharp
static Matrix TRS(Vec3 translation, Quat rotation, float scale)
```
Translate, Rotate, Scale. Creates a transform Matrix
using all these components!
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) translation|Move an object by this amount.|
|[Quat]({{site.url}}/Pages/Reference/Quat.html) rotation|A Quaternion describing the rotation for              this transform.|
|float scale|How much larger or smaller this transform              makes things. 1 is a good default, as 0 will shrink it to nothing!              This will expand to a scale vector of (size, size, size)|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|A Matrix that combines translation, rotation, and scale information into a single Matrix!|

<div class='signature' markdown='1'>
```csharp
static Matrix TRS(Vec3 translation, Quat rotation, Vec3 scale)
```
Translate, Rotate, Scale. Creates a transform Matrix
using all these components!
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) translation|Move an object by this amount.|
|[Quat]({{site.url}}/Pages/Reference/Quat.html) rotation|A Quaternion describing the rotation for              this transform.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) scale|How much larger or smaller this transform              makes things. Vec3.One is a good default, as Vec3.Zero will              shrink it to nothing!|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|A Matrix that combines translation, rotation, and scale information into a single Matrix!|

<div class='signature' markdown='1'>
```csharp
static Matrix TRS(Vec3 translation, Vec3 pitchYawRollDeg, float scale)
```
Translate, Rotate, Scale. Creates a transform Matrix
using all these components!
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) translation|Move an object by this amount.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) pitchYawRollDeg|Pitch (x-axis), yaw (y-axis), and              roll (z-axis) stored as x, y and z respectively in this Vec3.             Units are in degrees.|
|float scale|How much larger or smaller this transform              makes things. Vec3.One is a good default, as Vec3.Zero will              shrink it to nothing!|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|A Matrix that combines translation, rotation, and scale information into a single Matrix!|

<div class='signature' markdown='1'>
```csharp
static Matrix TRS(Vec3 translation, Vec3 pitchYawRollDeg, Vec3 scale)
```
Translate, Rotate, Scale. Creates a transform Matrix
using all these components!
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) translation|Move an object by this amount.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) pitchYawRollDeg|Pitch (x-axis), yaw (y-axis), and              roll (z-axis) stored as x, y and z respectively in this Vec3.             Units are in degrees.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) scale|How much larger or smaller this transform              makes things. Vec3.One is a good default, as Vec3.Zero will              shrink it to nothing!|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|A Matrix that combines translation, rotation, and scale information into a single Matrix!|




