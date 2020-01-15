---
layout: default
title: TexType
description: Textures come in various types and flavors! These are bit-flags that tell StereoKit what type of texture we want, and how the application might use it!
---
# TexType

Textures come in various types and flavors! These are bit-flags
that tell StereoKit what type of texture we want, and how the application
might use it!




## Static Fields and Properties

|  |  |
|--|--|
|[TexType]({{site.url}}/Pages/Reference/TexType.html) [Cubemap]({{site.url}}/Pages/Reference/TexType/Cubemap.html)|A size sided texture that's used for things like skyboxes, environment maps, and reflection probes. It behaves like a texture array with 6 textures.|
|[TexType]({{site.url}}/Pages/Reference/TexType.html) [Depth]({{site.url}}/Pages/Reference/TexType/Depth.html)|This texture contains depth data, not color data!|
|[TexType]({{site.url}}/Pages/Reference/TexType.html) [Dynamic]({{site.url}}/Pages/Reference/TexType/Dynamic.html)|This texture's data will be updated frequently from the CPU (not renders)! This ensures the graphics card stores it someplace where writes are easy to do quickly.|
|[TexType]({{site.url}}/Pages/Reference/TexType.html) [Image]({{site.url}}/Pages/Reference/TexType/Image.html)|A standard color image that also generates mip-maps automatically.|
|[TexType]({{site.url}}/Pages/Reference/TexType.html) [ImageNomips]({{site.url}}/Pages/Reference/TexType/ImageNomips.html)|A standard color image, without any generated mip-maps.|
|[TexType]({{site.url}}/Pages/Reference/TexType.html) [Mips]({{site.url}}/Pages/Reference/TexType/Mips.html)|This texture will generate mip-maps any time the contents change. Mip-maps are a list of textures that are each half the size of the one before them! This is used to prevent textures from 'sparkling' or aliasing in the distance.|
|[TexType]({{site.url}}/Pages/Reference/TexType.html) [Rendertarget]({{site.url}}/Pages/Reference/TexType/Rendertarget.html)|This texture can be rendered to! This is great for textures that might be passed in as a target to Renderer.Blit, or other such situations.|


