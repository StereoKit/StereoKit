---
layout: default
title: Hierarchy.ToWorldDirection
description: Converts a local direction relative to the current hierarchy stack into world space! This excludes the translation component normally applied to vectors, so it's still a valid direction.
---
# [Hierarchy]({{site.url}}/Pages/Reference/Hierarchy.html).ToWorldDirection

<div class='signature' markdown='1'>
```csharp
static Vec3 ToWorldDirection(Vec3 localDirection)
```
Converts a local direction relative to the current
hierarchy stack into world space! This excludes the translation
component normally applied to vectors, so it's still a valid
direction.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) localDirection|A direction in local space.|
|RETURNS: [Vec3]({{site.url}}/Pages/Reference/Vec3.html)|The provided direction now in world space!|




