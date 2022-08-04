---
layout: default
title: Cull
description: Culling is discarding an object from the render pipeline! This enum describes how mesh faces get discarded on the graphics card. With culling set to none, you can double the number of pixels the GPU ends up drawing, which can have a big impact on performance. None can be appropriate in cases where the mesh is designed to be 'double sided'. Front can also be helpful when you want to flip a mesh 'inside-out'!
---
# enum Cull

Culling is discarding an object from the render pipeline!
This enum describes how mesh faces get discarded on the graphics
card. With culling set to none, you can double the number of pixels
the GPU ends up drawing, which can have a big impact on performance.
None can be appropriate in cases where the mesh is designed to be
'double sided'. Front can also be helpful when you want to flip a
mesh 'inside-out'!

## Enum Values

|  |  |
|--|--|
|Back|Discard if the back of the triangle face is pointing towards the camera. This is the default behavior.|
|Front|Discard if the front of the triangle face is pointing towards the camera. This is opposite the default behavior.|
|None|No culling at all! Draw the triangle regardless of which way it's pointing.|
