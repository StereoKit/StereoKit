---
layout: default
title: MaterialBuffer`1.MaterialBuffer`1
description: Create a new global MaterialBuffer bound to the register slot id. All shaders will have access to the data provided via this instance's Set.
---
# [MaterialBuffer`1]({{site.url}}/Pages/Reference/MaterialBuffer`1.html).MaterialBuffer`1

<div class='signature' markdown='1'>
 MaterialBuffer`1(int registerSlot)
</div>

|  |  |
|--|--|
|int registerSlot|Valid values are 3-16. This is the              register id that this data will be bound to. In HLSL, you'll see             the slot id for '3' indicated like this `: register(b3)`|

Create a new global MaterialBuffer bound to the register
slot id. All shaders will have access to the data provided via
this instance's `Set`.



