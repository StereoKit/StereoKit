---
layout: default
title: Matrix.TR
description: Translate, Rotate. Creates a transform Matrix using these components!
---
# [Matrix]({{site.url}}/Pages/Reference/Matrix.html).TR

<div class='signature' markdown='1'>
```csharp
static Matrix TR(Vec3 translation, Quat rotation)
```
Translate, Rotate. Creates a transform Matrix using
these components!
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) translation|Move an object by this amount.|
|[Quat]({{site.url}}/Pages/Reference/Quat.html) rotation|A Quaternion describing the rotation for              this transform.|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|A Matrix that combines translation and rotation information into a single Matrix!|

<div class='signature' markdown='1'>
```csharp
static Matrix TR(float x, float y, float z, Quat rotation)
```
Translate, Rotate. Creates a transform Matrix using
these components!
</div>

|  |  |
|--|--|
|float x|Move an object on the x axis by this amount.|
|float y|Move an object on the y axis by this amount.|
|float z|Move an object on the z axis by this amount.|
|[Quat]({{site.url}}/Pages/Reference/Quat.html) rotation|A Quaternion describing the rotation for              this transform.|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|A Matrix that combines translation and rotation information into a single Matrix!|

<div class='signature' markdown='1'>
```csharp
static Matrix TR(Vec3 translation, Vec3 pitchYawRollDeg)
```
Translate, Rotate. Creates a transform Matrix using
these components!
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) translation|Move an object by this amount.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) pitchYawRollDeg|Pitch (x-axis), yaw (y-axis), and              roll (z-axis) stored as x, y and z respectively in this Vec3.             Units are in degrees.|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|A Matrix that combines translation and rotation information into a single Matrix!|

<div class='signature' markdown='1'>
```csharp
static Matrix TR(float x, float y, float z, Vec3 pitchYawRollDeg)
```
Translate, Rotate. Creates a transform Matrix using
these components!
</div>

|  |  |
|--|--|
|float x|Move an object on the x axis by this amount.|
|float y|Move an object on the y axis by this amount.|
|float z|Move an object on the z axis by this amount.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) pitchYawRollDeg|Pitch (x-axis), yaw (y-axis), and              roll (z-axis) stored as x, y and z respectively in this Vec3.             Units are in degrees.|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|A Matrix that combines translation and rotation information into a single Matrix!|




