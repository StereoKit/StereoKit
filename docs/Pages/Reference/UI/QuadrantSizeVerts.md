---
layout: default
title: UI.QuadrantSizeVerts
description: This will reposition the vertices to work well with quadrant resizing shaders. The mesh should generally be centered around the origin, and face down the -Z axis. This will also overwrite any UV coordinates in the verts.  You can read more about the technique [here](https.//playdeck.net/blog/quadrant-sizing-efficient-ui-rendering).
---
# [UI]({{site.url}}/Pages/Reference/UI.html).QuadrantSizeVerts

<div class='signature' markdown='1'>
static void QuadrantSizeVerts(Vertex[] verts, float overflowPercent)
</div>

|  |  |
|--|--|
|Vertex[] verts|A list of vertices to be modified to fit the             sizing shader.|
|float overflowPercent|When scaled, should the geometry             stick out past the "box" represented by the scale, or edge up             against it? A value of 0 will mean the geometry will fit entirely             inside the "box", and a value of 1 means the geometry will start at             the boundary of the box and continue outside it.|

This will reposition the vertices to work well with
quadrant resizing shaders. The mesh should generally be centered
around the origin, and face down the -Z axis. This will also
overwrite any UV coordinates in the verts.

You can read more about the technique [here](https://playdeck.net/blog/quadrant-sizing-efficient-ui-rendering).



