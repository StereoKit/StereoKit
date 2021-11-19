---
layout: default
title: Gradient.Get
description: Samples the gradient's color at the given position!
---
# [Gradient]({{site.url}}/Pages/Reference/Gradient.html).Get

<div class='signature' markdown='1'>
```csharp
Color Get(float at)
```
Samples the gradient's color at the given position!
</div>

|  |  |
|--|--|
|float at|Typically a value between 0-1, but if you used             larger or smaller values for your color key's positions, it'll be             in that range!|
|RETURNS: [Color]({{site.url}}/Pages/Reference/Color.html)|The interpolated color at the given position. If 'at' is smaller or larger than the gradient's position range, then the color will be clamped to the color at the beginning or end of the gradient!|




