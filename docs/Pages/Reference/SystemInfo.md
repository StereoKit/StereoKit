---
layout: default
title: SystemInfo
description: Information about a system's capabilities and properties!
---
# SystemInfo

Information about a system's capabilities and properties!


## Instance Fields and Properties

|  |  |
|--|--|
|int [displayHeight]({{site.url}}/Pages/Reference/SystemInfo/displayHeight.html)|Height of the display surface, in pixels! For a stereo display, this will be the height of a single eye.|
|[Display]({{site.url}}/Pages/Reference/Display.html) [displayType]({{site.url}}/Pages/Reference/SystemInfo/displayType.html)|The type of display this device has.|
|int [displayWidth]({{site.url}}/Pages/Reference/SystemInfo/displayWidth.html)|Width of the display surface, in pixels! For a stereo display, this will be the width of a single eye.|



## Static Fields and Properties

|  |  |
|--|--|
|bool [spatialBridge]({{site.url}}/Pages/Reference/SystemInfo/spatialBridge.html)|Does the device we're currently on have the spatial graph bridge extension? The extension is provided through the function `Pose.FromSpatialNode`. This allows OpenXR to talk with certain windows APIs, such as the QR code API that provides Graph Node GUIDs for the pose.|


