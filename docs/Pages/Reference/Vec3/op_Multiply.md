---
layout: default
title: Vec3.*
description: A component-wise vector multiplication, same thing as a non-uniform scale. NOT a dot or cross product! Commutative.
---
# [Vec3]({{site.url}}/Pages/Reference/Vec3.html).*

<div class='signature' markdown='1'>
```csharp
static Vec3 *(Vec3 a, Vec3 b)
```
A component-wise vector multiplication, same thing as
a non-uniform scale. NOT a dot or cross product! Commutative.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) a|Any vector.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) b|Any vector.|
|RETURNS: [Vec3]({{site.url}}/Pages/Reference/Vec3.html)|A new vector a scaled by b.|

<div class='signature' markdown='1'>
```csharp
static Vec3 *(Vec3 a, float b)
```
A scalar vector multiplication.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) a|Any vector.|
|float b|Any scalar.|
|RETURNS: [Vec3]({{site.url}}/Pages/Reference/Vec3.html)|A new vector a scaled by b.|

<div class='signature' markdown='1'>
```csharp
static Vec3 *(float a, Vec3 b)
```
A scalar vector multiplication.
</div>

|  |  |
|--|--|
|float a|Any scalar.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) b|Any vector.|
|RETURNS: [Vec3]({{site.url}}/Pages/Reference/Vec3.html)|A new vector a scaled by b.|




