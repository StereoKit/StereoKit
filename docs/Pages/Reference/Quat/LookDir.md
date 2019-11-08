---
layout: default
title: Quat.LookDir
description: Creates a rotation from a resting direction, to the given direction! This prevents roll on the Z axis, Up is always (0,1,0)
---
# [Quat]({{site.url}}/Pages/Reference/Quat.html).LookDir
<div class='signature' markdown='1'>
static [Quat]({{site.url}}/Pages/Reference/Quat.html) LookDir([Vec3]({{site.url}}/Pages/Reference/Vec3.html) direction)
</div>
Creates a rotation from a resting direction, to the given direction! This
prevents roll on the Z axis, Up is always (0,1,0)

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) direction|Direction the rotation should be looking. Doesn't need to be normalized.|
|RETURNS: [Quat]({{site.url}}/Pages/Reference/Quat.html)|A rotation from resting, to the given direction.|



