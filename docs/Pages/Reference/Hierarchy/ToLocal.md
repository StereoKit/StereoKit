---
layout: default
title: Hierarchy.ToLocal
description: Converts a world space point into the local space of the current Hierarchy stack!
---
# [Hierarchy]({{site.url}}/Pages/Reference/Hierarchy.html).ToLocal

<div class='signature' markdown='1'>
```csharp
static Vec3 ToLocal(Vec3 worldPoint)
```
Converts a world space point into the local space of the
current Hierarchy stack!
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) worldPoint|A point in world space.|
|RETURNS: [Vec3]({{site.url}}/Pages/Reference/Vec3.html)|The provided point now in local hierarchy space!|

<div class='signature' markdown='1'>
```csharp
static Quat ToLocal(Quat worldOrientation)
```
Converts a world space rotation into the local space of
the current Hierarchy stack!
</div>

|  |  |
|--|--|
|[Quat]({{site.url}}/Pages/Reference/Quat.html) worldOrientation|A rotation in world space.|
|RETURNS: [Quat]({{site.url}}/Pages/Reference/Quat.html)|The provided rotation now in local hierarchy space!|

<div class='signature' markdown='1'>
```csharp
static Pose ToLocal(Pose worldPose)
```
Converts a world pose relative to the current
hierarchy stack into local space!
</div>

|  |  |
|--|--|
|[Pose]({{site.url}}/Pages/Reference/Pose.html) worldPose|A pose in world space.|
|RETURNS: [Pose]({{site.url}}/Pages/Reference/Pose.html)|The provided pose now in world space!|




