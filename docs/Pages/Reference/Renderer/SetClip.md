---
layout: default
title: Renderer.SetClip
description: Set the near and far clipping planes of the camera! These are important to z-buffer quality, especially when using low bit depth z-buffers as recommended for devices like the HoloLens. The smaller the range between the near and far planes, the better your z-buffer will look! If you see flickering on objects that are overlapping, try making the range smaller.  These values only affect perspective mode projection, which is the default projection mode.
---
# [Renderer]({{site.url}}/Pages/Reference/Renderer.html).SetClip

<div class='signature' markdown='1'>
```csharp
static void SetClip(float nearPlane, float farPlane)
```
Set the near and far clipping planes of the camera!
These are important to z-buffer quality, especially when using
low bit depth z-buffers as recommended for devices like the
HoloLens. The smaller the range between the near and far planes,
the better your z-buffer will look! If you see flickering on
objects that are overlapping, try making the range smaller.

These values only affect perspective mode projection, which is the
default projection mode.
</div>

|  |  |
|--|--|
|float nearPlane|The GPU discards pixels that are too             close to the camera, this is that distance! It must be larger             than zero, due to the projection math, which also means that             numbers too close to zero will produce z-fighting artifacts. This             has an enforced minimum of 0.001, but you should probably stay             closer to 0.1.|
|float farPlane|At what distance from the camera does the             GPU discard pixel? This is not true distance, but rather Z-axis             distance from zero in View Space coordinates!|




