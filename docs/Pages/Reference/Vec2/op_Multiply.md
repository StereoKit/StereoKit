---
layout: default
title: Vec2.*
description: A component-wise vector multiplication, same thing as a non-uniform scale. NOT a dot product! Commutative.
---
# [Vec2]({{site.url}}/Pages/Reference/Vec2.html).*

<div class='signature' markdown='1'>
```csharp
static Vec2 *(Vec2 a, Vec2 b)
```
A component-wise vector multiplication, same thing as
a non-uniform scale. NOT a dot product! Commutative.
</div>

|  |  |
|--|--|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) a|Any vector.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) b|Any vector.|
|RETURNS: [Vec2]({{site.url}}/Pages/Reference/Vec2.html)|A new vector a scaled by b.|

<div class='signature' markdown='1'>
```csharp
static Vec2 *(Vec2 a, float b)
```
A scalar vector multiplication.
</div>

|  |  |
|--|--|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) a|Any vector.|
|float b|Any scalar.|
|RETURNS: [Vec2]({{site.url}}/Pages/Reference/Vec2.html)|A new vector a scaled by b.|

<div class='signature' markdown='1'>
```csharp
static Vec2 *(float a, Vec2 b)
```
A scalar vector multiplication.
</div>

|  |  |
|--|--|
|float a|Any scalar.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) b|Any vector.|
|RETURNS: [Vec2]({{site.url}}/Pages/Reference/Vec2.html)|A new vector a scaled by b.|




