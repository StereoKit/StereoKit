---
layout: default
title: UI.AffordanceBegin
description: This begins a new UI group with its own layout! Much like a window, except with a more flexible handle, and no header. You can draw the handle, but it will have no text on it.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).AffordanceBegin

<div class='signature' markdown='1'>
static bool AffordanceBegin(string id, Pose& pose, [Bounds]({{site.url}}/Pages/Reference/Bounds.html) handle, bool drawHandle, [UIMove]({{site.url}}/Pages/Reference/UIMove.html) moveType)
</div>

|  |  |
|--|--|
|string id|Id of the affordance group.|
|Pose& pose|The pose state for the affordance! The user will be able to grab              this affordance and move it around.|
|[Bounds]({{site.url}}/Pages/Reference/Bounds.html) handle|Size and location of the affordance handle, relative to the pose.|
|bool drawHandle|Should this function draw the handle for you, or will you             draw that yourself?|
|[UIMove]({{site.url}}/Pages/Reference/UIMove.html) moveType|Describes how the affordance will move when dragged around.|
|RETURNS: bool|Returns true for every frame the user is grabbing the handle.|

This begins a new UI group with its own layout! Much like a window, except
with a more flexible handle, and no header. You can draw the handle, but it will have
no text on it.



