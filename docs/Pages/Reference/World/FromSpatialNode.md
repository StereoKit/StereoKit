---
layout: default
title: World.FromSpatialNode
description: Converts a Windows Mirage spatial node GUID into a Pose based on its current position and rotation! Check SK.System.spatialBridgePresent to see if this is available to use. Currently only on HoloLens, good for use with the Windows QR code package.
---
# [World]({{site.url}}/Pages/Reference/World.html).FromSpatialNode

<div class='signature' markdown='1'>
```csharp
static Pose FromSpatialNode(Guid spatialNodeGuid, SpatialNodeType spatialNodeType, Int64 qpcTime)
```
Converts a Windows Mirage spatial node GUID into a Pose
based on its current position and rotation! Check
SK.System.spatialBridgePresent to see if this is available to
use. Currently only on HoloLens, good for use with the Windows
QR code package.
</div>

|  |  |
|--|--|
|Guid spatialNodeGuid|A Windows Mirage spatial node GUID             acquired from a windows MR API call.|
|SpatialNodeType spatialNodeType|Type of spatial node to locate.|
|Int64 qpcTime|A windows performance counter timestamp at             which the node should be located, obtained from another API or             with System.Diagnostics.Stopwatch.GetTimestamp().|
|RETURNS: [Pose]({{site.url}}/Pages/Reference/Pose.html)|A Pose representing the current orientation of the spatial node.|




