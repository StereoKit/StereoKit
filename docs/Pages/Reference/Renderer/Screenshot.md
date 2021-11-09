---
layout: default
title: Renderer.Screenshot
description: Schedules a screenshot for the end of the frame! The view will be rendered from the given position at the given point, with a resolution the same size as the screen's surface. It'll be saved as a .jpg file at the filename provided.
---
# [Renderer]({{site.url}}/Pages/Reference/Renderer.html).Screenshot

<div class='signature' markdown='1'>
```csharp
static void Screenshot(Vec3 from, Vec3 at, int width, int height, string filename)
```
Schedules a screenshot for the end of the frame! The view will be
rendered from the given position at the given point, with a resolution the same
size as the screen's surface. It'll be saved as a .jpg file at the filename
provided.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) from|Viewpoint location.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) at|Direction the viewpoint is looking at.|
|int width|Size of the screenshot horizontally, in pixels.|
|int height|Size of the screenshot vertically, in pixels.|
|string filename|Filename to write the screenshot to! Note this'll be a              .jpg regardless of what file extension you use right now.|

<div class='signature' markdown='1'>
```csharp
static void Screenshot(string filename, Vec3 from, Vec3 at, int width, int height, float fieldOfViewDegrees)
```
Schedules a screenshot for the end of the frame! The view
will be rendered from the given position at the given point, with a
resolution the same size as the screen's surface. It'll be saved as
a .jpg file at the filename provided.
</div>

|  |  |
|--|--|
|string filename|Filename to write the screenshot to! Note             this'll be a .jpg regardless of what file extension you use right             now.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) from|Viewpoint location.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) at|Direction the viewpoint is looking at.|
|int width|Size of the screenshot horizontally, in pixels.|
|int height|Size of the screenshot vertically, in pixels.|
|float fieldOfViewDegrees|The angle of the viewport, in              degrees.|




