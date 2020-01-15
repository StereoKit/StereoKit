---
layout: default
title: Vec3.Lerp
description: Blends (Linear Interpolation) between two vectors, based on a 'blend' value, where 0 is a, and 1 is b. Doesn't clamp percent for you.
---
# [Vec3]({{site.url}}/Pages/Reference/Vec3.html).Lerp

<div class='signature' markdown='1'>
static [Vec3]({{site.url}}/Pages/Reference/Vec3.html) Lerp(Vec3& a, Vec3& b, float blend)
</div>

|  |  |
|--|--|
|Vec3& a|First item in the blend, or '0.0' blend.|
|Vec3& b|Second item in the blend, or '1.0' blend.|
|float blend|A blend value between 0 and 1. Can be outside this range, it'll just             interpolate outside of the a, b range.|
|RETURNS: [Vec3]({{site.url}}/Pages/Reference/Vec3.html)|An unclamped blend of a and b.|

Blends (Linear Interpolation) between two vectors, based on a 'blend' value,
where 0 is a, and 1 is b. Doesn't clamp percent for you.




## Examples

```csharp
Vec3 position    = new Vec3(3, 2, 5);
Vec3 destination = new Vec3(3, 2, 8);
position = Vec3.Lerp(position, destination, 0.5f * Time.Elapsedf);
```

