---
layout: default
title: UI.QuadrantSizeMesh
description: This will reposition the Mesh's vertices to work well with quadrant resizing shaders. The mesh should generally be centered around the origin, and face down the -Z axis. This will also overwrite any UV coordinates in the verts.  You can read more about the technique [here](https.//playdeck.net/blog/quadrant-sizing-efficient-ui-rendering).
---
# [UI]({{site.url}}/Pages/Reference/UI.html).QuadrantSizeMesh

<div class='signature' markdown='1'>
```csharp
static void QuadrantSizeMesh(Mesh& mesh, float overflowPercent)
```
This will reposition the Mesh's vertices to work well with
quadrant resizing shaders. The mesh should generally be centered
around the origin, and face down the -Z axis. This will also
overwrite any UV coordinates in the verts.

You can read more about the technique [here](https://playdeck.net/blog/quadrant-sizing-efficient-ui-rendering).
</div>

|  |  |
|--|--|
|Mesh& mesh|The vertices of this Mesh will be retrieved,             modified, and overwritten.|
|float overflowPercent|When scaled, should the geometry             stick out past the "box" represented by the scale, or edge up             against it? A value of 0 will mean the geometry will fit entirely             inside the "box", and a value of 1 means the geometry will start at             the boundary of the box and continue outside it.|




