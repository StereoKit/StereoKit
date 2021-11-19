---
layout: default
title: Vec2.Angle
description: Returns the counter-clockwise degrees from [1,0]. Resulting value is between 0 and 360. Vector does not need to be normalized.
---
# [Vec2]({{site.url}}/Pages/Reference/Vec2.html).Angle

<div class='signature' markdown='1'>
```csharp
float Angle()
```
Returns the counter-clockwise degrees from [1,0].
Resulting value is between 0 and 360. Vector does not need to be
normalized.
</div>

|  |  |
|--|--|
|RETURNS: float|Counter-clockwise angle of the vector in degrees from [1,0], as a value between 0 and 360.|





## Examples

```csharp
Vec2 point = new Vec2(1, 0);
float angle0 = point.Angle();

point = new Vec2(0, 1);
float angle90 = point.Angle();

point = new Vec2(-1, 0);
float angle180 = point.Angle();

point = new Vec2(0, -1);
float angle270 = point.Angle();
```

