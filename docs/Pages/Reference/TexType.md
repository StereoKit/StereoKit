---
layout: default
title: TexType
description: Textures come in various types and flavors! These are bit-flags that tell StereoKit what type of texture we want, and how the application might use it!
---
# enum TexType

Textures come in various types and flavors! These are bit-flags
that tell StereoKit what type of texture we want, and how the application
might use it!

## Enum Values

|  |  |
|--|--|
|Cubemap|A size sided texture that's used for things like skyboxes, environment maps, and reflection probes. It behaves like a texture array with 6 textures.|
|Depth|This texture contains depth data, not color data!|
|Dynamic|This texture's data will be updated frequently from the CPU (not renders)! This ensures the graphics card stores it someplace where writes are easy to do quickly.|
|Image|A standard color image that also generates mip-maps automatically.|
|ImageNomips|A standard color image, without any generated mip-maps.|
|Mips|This texture will generate mip-maps any time the contents change. Mip-maps are a list of textures that are each half the size of the one before them! This is used to prevent textures from 'sparkling' or aliasing in the distance.|
|Rendertarget|This texture can be rendered to! This is great for textures that might be passed in as a target to Renderer.Blit, or other such situations.|
