---
layout: default
title: Cull
description: Culling is discarding an object from the render pipeline! This enum describes how mesh faces get discarded on the graphics card. With culling set to none, you can double the number of pixels the GPU ends up drawing, which can have a big impact on performance. None can be appropriate in cases where the mesh is designed to be 'double sided'. Front can also be helpful when you want to flip a mesh 'inside-out'!
---
# Cull

Culling is discarding an object from the render pipeline!
This enum describes how mesh faces get discarded on the graphics
card. With culling set to none, you can double the number of pixels
the GPU ends up drawing, which can have a big impact on performance.
None can be appropriate in cases where the mesh is designed to be
'double sided'. Front can also be helpful when you want to flip a
mesh 'inside-out'!




## Static Fields and Properties

|  |  |
|--|--|
|[Cull]({{site.url}}/Pages/Reference/Cull.html) [Back]({{site.url}}/Pages/Reference/Cull/Back.html)|Discard if the back of the triangle face is pointing towards the camera. This is the default behavior.|
|[Cull]({{site.url}}/Pages/Reference/Cull.html) [Front]({{site.url}}/Pages/Reference/Cull/Front.html)|Discard if the front of the triangle face is pointing towards the camera. This is opposite the default behavior.|
|[Cull]({{site.url}}/Pages/Reference/Cull.html) [None]({{site.url}}/Pages/Reference/Cull/None.html)|No culling at all! Draw the triangle regardless of which way it's pointing.|


