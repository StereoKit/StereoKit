---
layout: default
title: Pose.Lerp
description: Interpolates between two poses! t is unclamped, so values outside of (0,1) will extrapolate their position.
---
# [Pose]({{site.url}}/Pages/Reference/Pose.html).Lerp

<div class='signature' markdown='1'>
```csharp
static Pose Lerp(Pose a, Pose b, float percent)
```
Interpolates between two poses! t is unclamped, so values outside of (0,1) will
extrapolate their position.
</div>

|  |  |
|--|--|
|[Pose]({{site.url}}/Pages/Reference/Pose.html) a|Starting pose, or percent == 0|
|[Pose]({{site.url}}/Pages/Reference/Pose.html) b|Ending pose, or percent == 1|
|float percent|A value usually 0->1 that tells the blend between a and b.|
|RETURNS: [Pose]({{site.url}}/Pages/Reference/Pose.html)|A new pose, blended between a and b based on percent!|




