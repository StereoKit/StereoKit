---
layout: default
title: Pointer
description: Pointer is an abstraction of a number of different input sources, and a way to surface input events!
---
# struct Pointer

Pointer is an abstraction of a number of different input
sources, and a way to surface input events!

## Instance Fields and Properties

|  |  |
|--|--|
|[Quat]({{site.url}}/Pages/Reference/Quat.html) [orientation]({{site.url}}/Pages/Reference/Pointer/orientation.html)|Orientation of the pointer! Since a Ray has no concept of 'up', this can be used to retrieve more orientation information.|
|[Pose]({{site.url}}/Pages/Reference/Pose.html) [Pose]({{site.url}}/Pages/Reference/Pointer/Pose.html)|Convenience property that turns ray.position and orientation into a Pose.|
|[Ray]({{site.url}}/Pages/Reference/Ray.html) [ray]({{site.url}}/Pages/Reference/Pointer/ray.html)|A ray in the direction of the pointer.|
|[InputSource]({{site.url}}/Pages/Reference/InputSource.html) [source]({{site.url}}/Pages/Reference/Pointer/source.html)|What input source did this pointer come from? This is a bit-flag that contains input family and capability information.|
|[BtnState]({{site.url}}/Pages/Reference/BtnState.html) [state]({{site.url}}/Pages/Reference/Pointer/state.html)|What is the state of the input source's 'button', if it has one?|
|[BtnState]({{site.url}}/Pages/Reference/BtnState.html) [tracked]({{site.url}}/Pages/Reference/Pointer/tracked.html)|Is the pointer source being tracked right now?|
