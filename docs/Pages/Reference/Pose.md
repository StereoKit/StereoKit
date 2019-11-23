---
layout: default
title: Pose
description: Pose represents a location and orientation in space, excluding scale!
---
# Pose

Pose represents a location and orientation in space, excluding scale!


## Fields and Properties

|  |  |
|--|--|
|[Quat]({{site.url}}/Pages/Reference/Quat.html) [orientation]({{site.url}}/Pages/Reference/Pose/orientation.html)|Orientation of the pose, stored as a rotation from Vec3.Forward.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) [position]({{site.url}}/Pages/Reference/Pose/position.html)|Location of the pose.|



## Methods

|  |  |
|--|--|
|[Pose]({{site.url}}/Pages/Reference/Pose/Pose.html)|Basic initialization constructor! Just copies in the provided values directly.|
|[ToMatrix]({{site.url}}/Pages/Reference/Pose/ToMatrix.html)|Converts this pose into a transform matrix, incorporating a provided scale value.|


