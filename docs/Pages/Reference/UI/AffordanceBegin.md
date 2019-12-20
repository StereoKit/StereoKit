---
layout: default
title: UI.AffordanceBegin
description: This begins a new UI group with its own layout! Much like a window, except with a more flexible handle, and no header. You can draw the handle, but it will have no text on it.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).AffordanceBegin

<div class='signature' markdown='1'>
static bool AffordanceBegin(string id, Pose& pose, [Vec3]({{site.url}}/Pages/Reference/Vec3.html) handleCenter, [Vec3]({{site.url}}/Pages/Reference/Vec3.html) handleDimensions, bool drawHandle)
</div>

|  |  |
|--|--|
|string id|Id of the affordance group.|
|Pose& pose|The pose state for the affordance! The user will be able to grab              this affordance and move it around.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) handleCenter|Center of the affordance handle, relative to the pose.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) handleDimensions|Size of the affordance handle, in meters relative to the pose.|
|bool drawHandle|Should this function draw the handle for you, or will you             draw that yourself?|
|RETURNS: bool|Returns true for every frame the user is grabbing the handle.|

This begins a new UI group with its own layout! Much like a window, except
with a more flexible handle, and no header. You can draw the handle, but it will have
no text on it.



