---
layout: default
title: Vec2.Lerp
description: Blends (Linear Interpolation) between two vectors, based on a 'blend' value, where 0 is a, and 1 is b. Doesn't clamp percent for you.
---
# [Vec2]({{site.url}}/Pages/Reference/Vec2.html).Lerp

<div class='signature' markdown='1'>
```csharp
static Vec2 Lerp(Vec2 a, Vec2 b, float blend)
```
Blends (Linear Interpolation) between two vectors, based
on a 'blend' value, where 0 is a, and 1 is b. Doesn't clamp
percent for you.
</div>

|  |  |
|--|--|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) a|First item in the blend, or '0.0' blend.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) b|Second item in the blend, or '1.0' blend.|
|float blend|A blend value between 0 and 1. Can be outside             this range, it'll just interpolate outside of the a, b range.|
|RETURNS: [Vec2]({{site.url}}/Pages/Reference/Vec2.html)|An unclamped blend of a and b.|




