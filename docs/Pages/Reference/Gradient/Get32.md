---
layout: default
title: Gradient.Get32
description: Samples the gradient's color at the given position, and converts it to a 32 bit color. If your RGBA color values are outside of the 0-1 range, then you'll get some issues as they're converted to 0-255 range bytes!
---
# [Gradient]({{site.url}}/Pages/Reference/Gradient.html).Get32

<div class='signature' markdown='1'>
```csharp
Color32 Get32(float at)
```
Samples the gradient's color at the given position, and
converts it to a 32 bit color. If your RGBA color values are
outside of the 0-1 range, then you'll get some issues as they're
converted to 0-255 range bytes!
</div>

|  |  |
|--|--|
|float at|Typically a value between 0-1, but if you used             larger or smaller values for your color key's positions, it'll be             in that range!|
|RETURNS: [Color32]({{site.url}}/Pages/Reference/Color32.html)|The interpolated 32 bit color at the given position. If 'at' is smaller or larger than the gradient's position range, then the color will be clamped to the color at the beginning or end of the gradient!|




