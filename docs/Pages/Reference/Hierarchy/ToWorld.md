---
layout: default
title: Hierarchy.ToWorld
description: Converts a local point relative to the current hierarchy stack into world space!
---
# [Hierarchy]({{site.url}}/Pages/Reference/Hierarchy.html).ToWorld

<div class='signature' markdown='1'>
static [Vec3]({{site.url}}/Pages/Reference/Vec3.html) ToWorld([Vec3]({{site.url}}/Pages/Reference/Vec3.html) localPoint)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) localPoint|A point in local space.|
|RETURNS: [Vec3]({{site.url}}/Pages/Reference/Vec3.html)|The provided point now in world space!|

Converts a local point relative to the current hierarchy
stack into world space!
<div class='signature' markdown='1'>
static [Quat]({{site.url}}/Pages/Reference/Quat.html) ToWorld([Quat]({{site.url}}/Pages/Reference/Quat.html) localOrientation)
</div>

|  |  |
|--|--|
|[Quat]({{site.url}}/Pages/Reference/Quat.html) localOrientation|A rotation in local space.|
|RETURNS: [Quat]({{site.url}}/Pages/Reference/Quat.html)|The provided rotation now in world space!|

Converts a local rotation relative to the current
hierarchy stack into world space!
<div class='signature' markdown='1'>
static [Pose]({{site.url}}/Pages/Reference/Pose.html) ToWorld([Pose]({{site.url}}/Pages/Reference/Pose.html) localPose)
</div>

|  |  |
|--|--|
|[Pose]({{site.url}}/Pages/Reference/Pose.html) localPose|A pose in local space.|
|RETURNS: [Pose]({{site.url}}/Pages/Reference/Pose.html)|The provided pose now in world space!|

Converts a local pose relative to the current
hierarchy stack into world space!



