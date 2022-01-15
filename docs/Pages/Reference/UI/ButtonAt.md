---
layout: default
title: UI.ButtonAt
description: A variant of UI.Button that doesn't use the layout system, and instead goes exactly where you put it.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).ButtonAt

<div class='signature' markdown='1'>
```csharp
static bool ButtonAt(string text, Vec3 topLeftCorner, Vec2 size)
```
A variant of UI.Button that doesn't use the layout system,
and instead goes exactly where you put it.
</div>

|  |  |
|--|--|
|string text|Text to display on the button and id for             tracking element state. MUST be unique within current hierarchy.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) topLeftCorner|This is the top left corner of the UI             element relative to the current Hierarchy.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) size|The layout size for this element in Hierarchy             space. If an axis is left as zero, it will be auto-calculated. For             X this is the remaining width of the current layout, and for Y this             is UI.LineHeight.|
|RETURNS: bool|Will return true only on the first frame it is pressed!|




