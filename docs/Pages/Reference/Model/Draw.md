---
layout: default
title: Model.Draw
description: Adds this Model to the render queue for this frame! If the Hierarchy has a transform on it, that transform is combined with the Matrix provided here.
---
# [Model]({{site.url}}/Pages/Reference/Model.html).Draw

<div class='signature' markdown='1'>
void Draw([Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform, [Color]({{site.url}}/Pages/Reference/Color.html) color)
</div>

|  |  |
|--|--|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform|A Matrix that will transform the Model from Model Space into the current             Hierarchy Space.|
|[Color]({{site.url}}/Pages/Reference/Color.html) color|A per-instance color value to pass into the shader! Normally this gets used              like a material tint. If you're adventurous and don't need per-instance colors, this is a great              spot to pack in extra per-instance data for the shader!|

Adds this Model to the render queue for this frame! If the Hierarchy has a transform on it,
that transform is combined with the Matrix provided here.
<div class='signature' markdown='1'>
void Draw([Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform)
</div>

|  |  |
|--|--|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform|A Matrix that will transform the Model from Model Space into the current             Hierarchy Space.|

Adds this Model to the render queue for this frame! If the Hierarchy has a transform on it,
that transform is combined with the Matrix provided here.



