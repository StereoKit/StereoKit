---
layout: default
title: Mesh.Draw
description: Adds a mesh to the render queue for this frame! If the Hierarchy has a transform on it, that transform is combined with the Matrix provided here.
---
# [Mesh]({{site.url}}/Pages/Reference/Mesh.html).Draw

<div class='signature' markdown='1'>
void Draw([Material]({{site.url}}/Pages/Reference/Material.html) material, [Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform, [Color]({{site.url}}/Pages/Reference/Color.html) color)
</div>

|  |  |
|--|--|
|[Material]({{site.url}}/Pages/Reference/Material.html) material|A Material to apply to the Mesh.|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform|A Matrix that will transform the mesh from Model Space into the current             Hierarchy Space.|
|[Color]({{site.url}}/Pages/Reference/Color.html) color|A per-instance color value to pass into the shader! Normally this gets used              like a material tint. If you're adventurous and don't need per-instance colors, this is a great              spot to pack in extra per-instance data for the shader!|

Adds a mesh to the render queue for this frame! If the Hierarchy has a transform on it,
that transform is combined with the Matrix provided here.
<div class='signature' markdown='1'>
void Draw([Material]({{site.url}}/Pages/Reference/Material.html) material, [Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform)
</div>

|  |  |
|--|--|
|[Material]({{site.url}}/Pages/Reference/Material.html) material|A Material to apply to the Mesh.|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform|A Matrix that will transform the mesh from Model Space into the current             Hierarchy Space.|

Adds a mesh to the render queue for this frame! If the Hierarchy has a transform on it,
that transform is combined with the Matrix provided here.



