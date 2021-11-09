---
layout: default
title: SKMath.Lerp
description: Blends (Linear Interpolation) between two scalars, based on a 'blend' value, where 0 is a, and 1 is b. Doesn't clamp percent for you.
---
# [SKMath]({{site.url}}/Pages/Reference/SKMath.html).Lerp

<div class='signature' markdown='1'>
```csharp
static float Lerp(float a, float b, float t)
```
Blends (Linear Interpolation) between two scalars, based
on a 'blend' value, where 0 is a, and 1 is b. Doesn't clamp
percent for you.
</div>

|  |  |
|--|--|
|float a|First item in the blend, or '0.0' blend.|
|float b|Second item in the blend, or '1.0' blend.|
|float t|A blend value between 0 and 1. Can be outside             this range, it'll just interpolate outside of the a, b range.|
|RETURNS: float|An unclamped blend of a and b.|




