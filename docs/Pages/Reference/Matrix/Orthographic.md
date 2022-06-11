---
layout: default
title: Matrix.Orthographic
description: This creates a matrix used for projecting 3D geometry onto a 2D surface for rasterization. Orthographic projection matrices will preserve parallel lines. This is great for 2D scenes or content.
---
# [Matrix]({{site.url}}/Pages/Reference/Matrix.html).Orthographic

<div class='signature' markdown='1'>
```csharp
static Matrix Orthographic(float width, float height, float nearClip, float farClip)
```
This creates a matrix used for projecting 3D geometry
onto a 2D surface for rasterization. Orthographic projection
matrices will preserve parallel lines. This is great for 2D
scenes or content.
</div>

|  |  |
|--|--|
|float width|The width, in meters, of the area that will              be projected.|
|float height|The height, in meters, of the area that will             be projected.|
|float nearClip|Anything closer than this distance (in             meters) will be discarded. Must not be zero, and if you make this             too small, you may experience glitching in your depth buffer.|
|float farClip|Anything further than this distance (in             meters) will be discarded. For low resolution depth buffers, this             should not be too far away, or you'll see bad z-fighting              artifacts.|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|The final orthographic matrix.|




