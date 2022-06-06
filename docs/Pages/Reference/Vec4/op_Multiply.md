---
layout: default
title: Vec4.*
description: A component-wise vector multiplication, same thing as a non-uniform scale. NOT a dot product! Commutative.
---
# [Vec4]({{site.url}}/Pages/Reference/Vec4.html).*

<div class='signature' markdown='1'>
```csharp
static Vec4 *(Vec4 a, Vec4 b)
```
A component-wise vector multiplication, same thing as
a non-uniform scale. NOT a dot product! Commutative.
</div>

|  |  |
|--|--|
|[Vec4]({{site.url}}/Pages/Reference/Vec4.html) a|Any vector.|
|[Vec4]({{site.url}}/Pages/Reference/Vec4.html) b|Any vector.|
|RETURNS: [Vec4]({{site.url}}/Pages/Reference/Vec4.html)|A new vector a scaled by b.|

<div class='signature' markdown='1'>
```csharp
static Vec4 *(Vec4 a, float b)
```
A scalar vector multiplication.
</div>

|  |  |
|--|--|
|[Vec4]({{site.url}}/Pages/Reference/Vec4.html) a|Any vector.|
|float b|Any scalar.|
|RETURNS: [Vec4]({{site.url}}/Pages/Reference/Vec4.html)|A new vector a scaled by b.|

<div class='signature' markdown='1'>
```csharp
static Vec4 *(float a, Vec4 b)
```
A scalar vector multiplication.
</div>

|  |  |
|--|--|
|float a|Any scalar.|
|[Vec4]({{site.url}}/Pages/Reference/Vec4.html) b|Any vector.|
|RETURNS: [Vec4]({{site.url}}/Pages/Reference/Vec4.html)|A new vector a scaled by b.|




