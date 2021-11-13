---
layout: default
title: UI.LayoutArea
description: Manually define what area is used for the UI layout. This is in the current Hierarchy's coordinate space on the X/Y plane.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).LayoutArea

<div class='signature' markdown='1'>
```csharp
static void LayoutArea(Vec3 start, Vec2 dimensions)
```
Manually define what area is used for the UI layout. This
is in the current Hierarchy's coordinate space on the X/Y plane.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) start|The top left of the layout area, relative to             the current Hierarchy in local meters.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) dimensions|The size of the layout area from the top             left, in local meters.|




