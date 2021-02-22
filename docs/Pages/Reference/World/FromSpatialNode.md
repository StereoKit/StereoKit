---
layout: default
title: World.FromSpatialNode
description: Converts a Windows Mirage spatial node GUID into a Pose based on its current position and rotation! Check StereoKitApp.System.spatialBridge to see if this is available to use. Currently only on HoloLens, good for use with the Windows QR code package.
---
# [World]({{site.url}}/Pages/Reference/World.html).FromSpatialNode

<div class='signature' markdown='1'>
static [Pose]({{site.url}}/Pages/Reference/Pose.html) FromSpatialNode(Guid spatialNodeGuid)
</div>

|  |  |
|--|--|
|Guid spatialNodeGuid|A Windows Mirage spatial node GUID             aquired from a windows MR API call.|
|RETURNS: [Pose]({{site.url}}/Pages/Reference/Pose.html)|A Pose representing the current orientation of the spatial node.|

Converts a Windows Mirage spatial node GUID into a Pose
based on its current position and rotation! Check
StereoKitApp.System.spatialBridge to see if this is available to
use. Currently only on HoloLens, good for use with the Windows
QR code package.



