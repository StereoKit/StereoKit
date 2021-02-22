---
layout: default
title: Input.HandSolid
description: Does StereoKit register the hand with the physics system? By default, this is true. Right now this is just a single block collider, but later will involve per-joint colliders!
---
# [Input]({{site.url}}/Pages/Reference/Input.html).HandSolid

<div class='signature' markdown='1'>
static void HandSolid([Handed]({{site.url}}/Pages/Reference/Handed.html) hand, bool solid)
</div>

|  |  |
|--|--|
|[Handed]({{site.url}}/Pages/Reference/Handed.html) hand|If Handed.Max, this will set the value for              both hands.|
|bool solid|True? Physics! False? No physics.|

Does StereoKit register the hand with the physics
system? By default, this is true. Right now this is just a single
block collider, but later will involve per-joint colliders!



