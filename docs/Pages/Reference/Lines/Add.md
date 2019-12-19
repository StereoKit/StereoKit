---
layout: default
title: Lines.Add
description: Adds a line to the environment for the current frame.
---
# [Lines]({{site.url}}/Pages/Reference/Lines.html).Add

<div class='signature' markdown='1'>
static void Add([Vec3]({{site.url}}/Pages/Reference/Vec3.html) start, [Vec3]({{site.url}}/Pages/Reference/Vec3.html) end, [Color32]({{site.url}}/Pages/Reference/Color32.html) color, float thickness)
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) start|Starting point of the line.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) end|End point of the line.|
|[Color32]({{site.url}}/Pages/Reference/Color32.html) color|Color for the line, this is embedded in the vertex color of the line.|
|float thickness|Thickness of the line in meters.|

Adds a line to the environment for the current frame.
<div class='signature' markdown='1'>
static void Add([Ray]({{site.url}}/Pages/Reference/Ray.html) ray, float length, [Color32]({{site.url}}/Pages/Reference/Color32.html) color, float thickness)
</div>

|  |  |
|--|--|
|[Ray]({{site.url}}/Pages/Reference/Ray.html) ray|The ray we want to visualize!|
|float length|How long should the ray be? Actual length will be ray.direction.Magnitude * length.|
|[Color32]({{site.url}}/Pages/Reference/Color32.html) color|Color for the line, this is embedded in the vertex color of the line.|
|float thickness|Thickness of the line in meters.|

Adds a line based on a ray to the environment for the current frame.



