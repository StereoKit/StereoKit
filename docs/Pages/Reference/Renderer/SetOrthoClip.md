---
layout: default
title: Renderer.SetOrthoClip
description: Set the near and far clipping planes of the camera! These are important to z-buffer quality, especially when using low bit depth z-buffers as recommended for devices like the HoloLens. The smaller the range between the near and far planes, the better your z-buffer will look! If you see flickering on objects that are overlapping, try making the range smaller.  These values only affect orthographic mode projection, which is only available in flatscreen.
---
# [Renderer]({{site.url}}/Pages/Reference/Renderer.html).SetOrthoClip

<div class='signature' markdown='1'>
```csharp
static void SetOrthoClip(float nearPlane, float farPlane)
```
Set the near and far clipping planes of the camera!
These are important to z-buffer quality, especially when using
low bit depth z-buffers as recommended for devices like the
HoloLens. The smaller the range between the near and far planes,
the better your z-buffer will look! If you see flickering on
objects that are overlapping, try making the range smaller.

These values only affect orthographic mode projection, which is
only available in flatscreen.
</div>

|  |  |
|--|--|
|float nearPlane|The GPU discards pixels that are too             close to the camera, this is that distance!|
|float farPlane|At what distance from the camera does the             GPU discard pixel? This is not true distance, but rather Z-axis             distance from zero in View Space coordinates!|




