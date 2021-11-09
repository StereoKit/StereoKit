---
layout: default
title: Matrix.Perspective
description: This creates a matrix used for projecting 3D geometry onto a 2D surface for rasterization. Perspective projection matrices will cause parallel lines to converge at the horizon. This is great for normal looking content.
---
# [Matrix]({{site.url}}/Pages/Reference/Matrix.html).Perspective

<div class='signature' markdown='1'>
```csharp
static Matrix Perspective(float fovDegrees, float aspectRatio, float nearClip, float farClip)
```
This creates a matrix used for projecting 3D geometry
onto a 2D surface for rasterization. Perspective projection
matrices will cause parallel lines to converge at the horizon.
This is great for normal looking content.
</div>

|  |  |
|--|--|
|float fovDegrees|This is the vertical field of view of             the perspective matrix, units are in degrees.|
|float aspectRatio|The projection surface's width/height.|
|float nearClip|Anything closer than this distance (in             meters) will be discarded. Must not be zero, and if you make this             too small, you may experience glitching in your depth buffer.|
|float farClip|Anything further than this distance (in             meters) will be discarded. For low resolution depth buffers, this             should not be too far away, or you'll see bad z-fighting              artifacts.|
|RETURNS: [Matrix]({{site.url}}/Pages/Reference/Matrix.html)|The final perspective matrix.|




