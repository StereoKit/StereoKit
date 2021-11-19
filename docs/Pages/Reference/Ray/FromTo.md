---
layout: default
title: Ray.FromTo
description: A convenience function that creates a ray from point a, towards point b. Resulting direction is not normalized.
---
# [Ray]({{site.url}}/Pages/Reference/Ray.html).FromTo

<div class='signature' markdown='1'>
```csharp
static Ray FromTo(Vec3 a, Vec3 b)
```
A convenience function that creates a ray from point a,
towards point b. Resulting direction is not normalized.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) a|Ray starting point.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) b|Location the ray is pointing towards.|
|RETURNS: [Ray]({{site.url}}/Pages/Reference/Ray.html)|A ray from point a to point b. Not normalized.|




