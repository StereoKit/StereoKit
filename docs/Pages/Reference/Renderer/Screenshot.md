---
layout: default
title: Renderer.Screenshot
description: Schedules a screenshot for the end of the frame! The view will be rendered from the given position at the given point, with a resolution the same size as the screen's surface. It'll be saved as a .jpg file at the filename provided.
---
# [Renderer]({{site.url}}/Pages/Reference/Renderer.html).Screenshot

<div class='signature' markdown='1'>
static void Screenshot([Vec3]({{site.url}}/Pages/Reference/Vec3.html) from, [Vec3]({{site.url}}/Pages/Reference/Vec3.html) at, string filename)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) from|Viewpoint location.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) at|Direction the viewpoint is looking at.|
|string filename|Filename to write the screenshot to! Note this'll be a              .jpg regardless of what file extension you use right now.|

Schedules a screenshot for the end of the frame! The view will be
rendered from the given position at the given point, with a resolution the same
size as the screen's surface. It'll be saved as a .jpg file at the filename
provided.



