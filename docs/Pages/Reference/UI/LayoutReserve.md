---
layout: default
title: UI.LayoutReserve
description: Reserves a box of space for an item in the current UI layout! If either size axis is zero, it will be auto-sized to fill the current surface horizontally, and fill a single LineHeight vertically. Returns the Hierarchy local bounds of the space that was reserved, with a Z axis dimension of 0.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).LayoutReserve

<div class='signature' markdown='1'>
```csharp
static Bounds LayoutReserve(Vec2 size, bool addPadding, float depth)
```
Reserves a box of space for an item in the current UI
layout! If either size axis is zero, it will be auto-sized to fill
the current surface horizontally, and fill a single LineHeight
vertically. Returns the Hierarchy local bounds of the space that
was reserved, with a Z axis dimension of 0.
</div>

|  |  |
|--|--|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) size|Size of the layout box in Hierarchy local             meters.|
|bool addPadding|If true, this will add the current padding             value to the total final dimensions of the space that is reserved.|
|float depth|This allows you to quickly insert a depth into             the Bounds you're receiving. This will offset on the Z axis in             addition to increasing the dimensions, so that the bounds still             remain sitting on the surface of the UI.                          This depth value will not be reflected in the bounds provided by              LayouLast.|
|RETURNS: [Bounds]({{site.url}}/Pages/Reference/Bounds.html)|Returns the Hierarchy local bounds of the space that was reserved, with a Z axis dimension of 0.|




