---
layout: default
title: Renderer.CameraRoot
description: Sets and gets the root transform of the camera! This will be the identity matrix by default. The user's head  location will then be relative to this point. This is great to use if you're trying to do teleportation, redirected walking, or just shifting the floor around.
---
# [Renderer]({{site.url}}/Pages/Reference/Renderer.html).CameraRoot

<div class='signature' markdown='1'>
static [Matrix]({{site.url}}/Pages/Reference/Matrix.html) CameraRoot{ get set }
</div>

## Description
Sets and gets the root transform of the camera! This
will be the identity matrix by default. The user's head  location
will then be relative to this point. This is great to use if
you're trying to do teleportation, redirected walking, or just
shifting the floor around.

