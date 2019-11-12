---
layout: default
title: Quat.LookAt
description: Creates a rotation from a resting direction, to a direction indicated by the direction of the two vectors provided to the function! This is a great function for camera style rotation, when you know where a camera is, and where you want to look at. This prevents roll on the Z axis, Up is always (0,1,0).
---
# [Quat]({{site.url}}/Pages/Reference/Quat.html).LookAt

<div class='signature' markdown='1'>
static [Quat]({{site.url}}/Pages/Reference/Quat.html) LookAt([Vec3]({{site.url}}/Pages/Reference/Vec3.html) from, [Vec3]({{site.url}}/Pages/Reference/Vec3.html) to)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) from|Where the object is.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) to|Where the object should be looking!|
|RETURNS: [Quat]({{site.url}}/Pages/Reference/Quat.html)|A rotation from resting, to the look direction of the parameters.|

Creates a rotation from a resting direction, to a direction indicated
by the direction of the two vectors provided to the function! This is a great
function for camera style rotation, when you know where a camera is, and where
you want to look at. This prevents roll on the Z axis, Up is always (0,1,0).



