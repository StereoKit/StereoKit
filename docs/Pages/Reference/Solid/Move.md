---
layout: default
title: Solid.Move
description: This moves the Solid from its current location through space to the new location provided, colliding with things along the way. This is achieved by applying the velocity and angular velocity necessary to get to the destination in a single frame during the next physics step, then restoring the previous velocity info afterwards! See also Teleport for movement without collision.
---
# [Solid]({{site.url}}/Pages/Reference/Solid.html).Move

<div class='signature' markdown='1'>
```csharp
void Move(Vec3 position, Quat rotation)
```
This moves the Solid from its current location through
space to the new location provided, colliding with things along
the way. This is achieved by applying the velocity and angular
velocity necessary to get to the destination in a single frame
during the next physics step, then restoring the previous
velocity info afterwards! See also Teleport for movement without
collision.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) position|The destination position!|
|[Quat]({{site.url}}/Pages/Reference/Quat.html) rotation|The destination rotation!|




