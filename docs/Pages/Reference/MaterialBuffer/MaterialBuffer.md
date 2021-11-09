---
layout: default
title: MaterialBuffer.MaterialBuffer
description: Create a new global MaterialBuffer bound to the register slot id. All shaders will have access to the data provided via this instance's Set.
---
# [MaterialBuffer]({{site.url}}/Pages/Reference/MaterialBuffer.html).MaterialBuffer

<div class='signature' markdown='1'>
```csharp
void MaterialBuffer(int registerSlot)
```
Create a new global MaterialBuffer bound to the register
slot id. All shaders will have access to the data provided via
this instance's `Set`.
</div>

|  |  |
|--|--|
|int registerSlot|Valid values are 3-16. This is the              register id that this data will be bound to. In HLSL, you'll see             the slot id for '3' indicated like this `: register(b3)`|




