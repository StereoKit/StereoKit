---
layout: default
title: Renderer.SetView
description: Sets the position of the camera! This only works for Flatscreen experiences, as the MR runtime will control the camera when active.
---
# [Renderer]({{site.url}}/Pages/Reference/Renderer.html).SetView

<div class='signature' markdown='1'>
static void SetView([Matrix]({{site.url}}/Pages/Reference/Matrix.html) cameraTransform)
</div>

|  |  |
|--|--|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) cameraTransform|A transform matrix representing a transform to the camera's              orientation! This is not the final view transform, StereoKit will invert this matrix for use              in the transform into view space. A regular Matrix.TRS call will work great here!|

Sets the position of the camera! This only works for Flatscreen experiences, as
the MR runtime will control the camera when active.



