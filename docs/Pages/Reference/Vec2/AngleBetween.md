---
layout: default
title: Vec2.AngleBetween
description: Calculates a signed angle between two vectors! Sign will be positive if B is counter-clockwise (left) of A, and negative if B is clockwise (right) of A. Vectors do not need to be normalized.
---
# [Vec2]({{site.url}}/Pages/Reference/Vec2.html).AngleBetween

<div class='signature' markdown='1'>
```csharp
static float AngleBetween(Vec2 a, Vec2 b)
```
Calculates a signed angle between two vectors! Sign will
be positive if B is counter-clockwise (left) of A, and negative
if B is clockwise (right) of A. Vectors do not need to be
normalized.
</div>

|  |  |
|--|--|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) a|The first, initial vector, A. Does not need to be             normalized.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) b|The second vector, B, that we're finding the              angle to. Does not need to be normalized.|
|RETURNS: float|a signed angle between two vectors! Sign will be positive if B is counter-clockwise (left) of A, and negative if B is clockwise (right) of A.|





## Examples

```csharp
Vec2 directionA = new Vec2( 1, 1);
Vec2 directionB = new Vec2(-1, 1);
float angle90 = Vec2.AngleBetween(directionA, directionB);

directionA = new Vec2(1, 1);
directionB = new Vec2(0,-2);
float angleNegative135 = Vec2.AngleBetween(directionA, directionB);
```

