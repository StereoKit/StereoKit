---
layout: default
title: Hierarchy.ToWorld
description: Converts a local point relative to the current hierarchy stack into world space!
---
# [Hierarchy]({{site.url}}/Pages/Reference/Hierarchy.html).ToWorld

<div class='signature' markdown='1'>
```csharp
static Vec3 ToWorld(Vec3 localPoint)
```
Converts a local point relative to the current hierarchy
stack into world space!
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) localPoint|A point in local space.|
|RETURNS: [Vec3]({{site.url}}/Pages/Reference/Vec3.html)|The provided point now in world space!|

<div class='signature' markdown='1'>
```csharp
static Quat ToWorld(Quat localOrientation)
```
Converts a local rotation relative to the current
hierarchy stack into world space!
</div>

|  |  |
|--|--|
|[Quat]({{site.url}}/Pages/Reference/Quat.html) localOrientation|A rotation in local space.|
|RETURNS: [Quat]({{site.url}}/Pages/Reference/Quat.html)|The provided rotation now in world space!|

<div class='signature' markdown='1'>
```csharp
static Pose ToWorld(Pose localPose)
```
Converts a local pose relative to the current
hierarchy stack into world space!
</div>

|  |  |
|--|--|
|[Pose]({{site.url}}/Pages/Reference/Pose.html) localPose|A pose in local space.|
|RETURNS: [Pose]({{site.url}}/Pages/Reference/Pose.html)|The provided pose now in world space!|




