---
layout: default
title: Hierarchy.Push
description: Pushes a transform Matrix onto the stack, and combines it with the Matrix below it. Any draw operation's Matrix will now be combined with this Matrix to make it relative to the current hierarchy. Use Hierarchy.Pop to remove it from the Hierarchy stack! All Push calls must have an accompanying Pop call.
---
# [Hierarchy]({{site.url}}/Pages/Reference/Hierarchy.html).Push

<div class='signature' markdown='1'>
```csharp
static void Push(Matrix& parentTransform)
```
Pushes a transform Matrix onto the stack, and combines
it with the Matrix below it. Any draw operation's Matrix will now
be combined with this Matrix to make it relative to the current
hierarchy. Use Hierarchy.Pop to remove it from the Hierarchy
stack! All Push calls must have an accompanying Pop call.
</div>

|  |  |
|--|--|
|Matrix& parentTransform|The transform Matrix you want to              apply to all following draw calls.|




