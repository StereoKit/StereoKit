---
layout: default
title: Gradient.Set
description: Updates the color key at the given index! This will NOT re-order color keys if they are moved past another key's position, which could lead to strange behavior.
---
# [Gradient]({{site.url}}/Pages/Reference/Gradient.html).Set

<div class='signature' markdown='1'>
```csharp
void Set(int index, Color colorLinear, float position)
```
Updates the color key at the given index! This will NOT
re-order color keys if they are moved past another key's position,
which could lead to strange behavior.
</div>

|  |  |
|--|--|
|int index|Index of the color key to change.|
|[Color]({{site.url}}/Pages/Reference/Color.html) colorLinear|Any linear space color you like!|
|float position|Typically a value between 0-1! This is the              position of the color along the 'x-axis' of the gradient.|




