---
layout: default
title: Pose.Pose
description: Basic initialization constructor! Just copies in the provided values directly.
---
# [Pose]({{site.url}}/Pages/Reference/Pose.html).Pose

<div class='signature' markdown='1'>
```csharp
void Pose(Vec3 position, Quat orientation)
```
Basic initialization constructor! Just copies in the provided values directly.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) position|Location of the pose.|
|[Quat]({{site.url}}/Pages/Reference/Quat.html) orientation|Orientation of the pose, stored as a rotation from Vec3.Forward.|

<div class='signature' markdown='1'>
```csharp
void Pose(float x, float y, float z, Quat orientation)
```
Basic initialization constructor! Just copies in the provided values directly.
</div>

|  |  |
|--|--|
|float x|X location of the pose.|
|float y|Y location of the pose.|
|float z|Z location of the pose.|
|[Quat]({{site.url}}/Pages/Reference/Quat.html) orientation|Orientation of the pose, stored as a rotation from Vec3.Forward.|




