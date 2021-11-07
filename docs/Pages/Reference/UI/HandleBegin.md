---
layout: default
title: UI.HandleBegin
description: This begins a new UI group with its own layout! Much like a window, except with a more flexible handle, and no header. You can draw the handle, but it will have no text on it. Returns true for every frame the user is grabbing the handle.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).HandleBegin

<div class='signature' markdown='1'>
static bool HandleBegin(string id, Pose& pose, [Bounds]({{site.url}}/Pages/Reference/Bounds.html) handle, bool drawHandle, [UIMove]({{site.url}}/Pages/Reference/UIMove.html) moveType)
</div>

|  |  |
|--|--|
|string id|An id for tracking element state. MUST be unique             within current hierarchy.|
|Pose& pose|The pose state for the handle! The user will              be able to grab this handle and move it around.|
|[Bounds]({{site.url}}/Pages/Reference/Bounds.html) handle|Size and location of the handle, relative to              the pose.|
|bool drawHandle|Should this function draw the handle              visual for you, or will you draw that yourself?|
|[UIMove]({{site.url}}/Pages/Reference/UIMove.html) moveType|Describes how the handle will move when              dragged around.|
|RETURNS: bool|Returns true for every frame the user is grabbing the handle.|

This begins a new UI group with its own layout! Much
like a window, except with a more flexible handle, and no header.
You can draw the handle, but it will have no text on it. Returns
true for every frame the user is grabbing the handle.



