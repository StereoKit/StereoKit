---
layout: default
title: SystemInfo.spatialBridgePresent
description: Does the device we're currently on have the spatial graph bridge extension? The extension is provided through the function World.FromSpatialNode. This allows OpenXR to talk with certain windows APIs, such as the QR code API that provides Graph Node GUIDs for the pose.
---
# [SystemInfo]({{site.url}}/Pages/Reference/SystemInfo.html).spatialBridgePresent

<div class='signature' markdown='1'>
bool spatialBridgePresent{ get }
</div>

## Description
Does the device we're currently on have the spatial
graph bridge extension? The extension is provided through the
function `World.FromSpatialNode`. This allows OpenXR to talk with
certain windows APIs, such as the QR code API that provides Graph
Node GUIDs for the pose.

