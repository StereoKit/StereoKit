---
layout: default
title: Matrix.R
description: Create a rotation matrix from a Quaternion.
---
# [Matrix]({{site.url}}/Pages/Reference/Matrix.html).R

<div class='signature' markdown='1'>
```csharp
static Matrix R(Quat rotation)
```
Create a rotation matrix from a Quaternion.
</div>

|  |  |
|--|--|
|[Quat]({{site.url}}/Pages/Reference/Quat.html) rotation|A Quaternion describing the rotation for              this transform.|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|A Matrix that will rotate by the provided Quaternion orientation.|

<div class='signature' markdown='1'>
```csharp
static Matrix R(float pitchXDeg, float yawYDeg, float rollZDeg)
```
Create a rotation matrix from pitch, yaw, and roll
information. Units are in degrees.
</div>

|  |  |
|--|--|
|float pitchXDeg|Pitch, or rotation around the X axis, in             degrees.|
|float yawYDeg|Yaw, or rotation around the Y axis, in              degrees.|
|float rollZDeg|Roll, or rotation around the Z axis, in             degrees.|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|A Matrix that will rotate by the provided pitch, yaw and roll.|

<div class='signature' markdown='1'>
```csharp
static Matrix R(Vec3 pitchYawRollDeg)
```
Create a rotation matrix from pitch, yaw, and roll
information. Units are in degrees.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) pitchYawRollDeg|Pitch (x-axis), yaw (y-axis), and              roll (z-axis) stored as x, y and z respectively in this Vec3.             Units are in degrees.|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|A Matrix that will rotate by the provided pitch, yaw and roll.|




