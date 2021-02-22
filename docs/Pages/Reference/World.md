---
layout: default
title: World
description: World contains information about the real world around the user. This includes things like play boundaries, scene understanding, and other various things.
---
# World

World contains information about the real world around the
user. This includes things like play boundaries, scene understanding,
and other various things.




## Static Fields and Properties

|  |  |
|--|--|
|[Pose]({{site.url}}/Pages/Reference/Pose.html) [BoundsPose]({{site.url}}/Pages/Reference/World/BoundsPose.html)|This is the orientation and center point of the system's boundary/guardian. This can be useful to find the floor height! Not all systems have a boundary, so be sure to check `World.HasBounds` first.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [BoundsSize]({{site.url}}/Pages/Reference/World/BoundsSize.html)|This is the size of a rectangle within the play boundary/guardian's space, in meters if one exists. Check `World.BoundsPose` for the center point and orientation of the boundary, and check `World.HasBounds` to see if it exists at all!|
|bool [HasBounds]({{site.url}}/Pages/Reference/World/HasBounds.html)|This refers to the play boundary, or guardian system that the system may have! Not all systems have this, so it's always a good idea to check this first!|


## Static Methods

|  |  |
|--|--|
|[FromSpatialNode]({{site.url}}/Pages/Reference/World/FromSpatialNode.html)|Converts a Windows Mirage spatial node GUID into a Pose based on its current position and rotation! Check StereoKitApp.System.spatialBridge to see if this is available to use. Currently only on HoloLens, good for use with the Windows QR code package.|

