---
layout: default
title: TexFormat
description: What type of color information will the texture contain? A good default here is Rgba32.
---
# TexFormat

What type of color information will the texture contain? A
good default here is Rgba32.




## Static Fields and Properties

|  |  |
|--|--|
|[TexFormat]({{site.url}}/Pages/Reference/TexFormat.html) [Depth16]({{site.url}}/Pages/Reference/TexFormat/Depth16.html)|16 bits of depth is not a lot, but it can be enough if your far clipping plane is pretty close. If you're seeing lots of flickering where two objects overlap, you either need to bring your far clip in, or switch to 32/24 bit depth.|
|[TexFormat]({{site.url}}/Pages/Reference/TexFormat.html) [Depth32]({{site.url}}/Pages/Reference/TexFormat/Depth32.html)|32 bits of data per depth value! This is pretty detailed, and is excellent for experiences that have a very far view distance.|
|[TexFormat]({{site.url}}/Pages/Reference/TexFormat.html) [DepthStencil]({{site.url}}/Pages/Reference/TexFormat/DepthStencil.html)|A depth data format, 24 bits for depth data, and 8 bits to store stencil information! Stencil data can be used for things like clipping effects, deferred rendering, or shadow effects.|
|[TexFormat]({{site.url}}/Pages/Reference/TexFormat.html) [None]({{site.url}}/Pages/Reference/TexFormat/None.html)|A default zero value for TexFormat! Unitialized formats will get this value and **** **** up so you know to assign it properly :)|
|[TexFormat]({{site.url}}/Pages/Reference/TexFormat.html) [R16]({{site.url}}/Pages/Reference/TexFormat/R16.html)|A single channel of data, with 16 bits per-pixel! This is a good format for height maps, since it stores a fair bit of information in it. Values in the shader are always 0.0-1.0.|
|[TexFormat]({{site.url}}/Pages/Reference/TexFormat.html) [R32]({{site.url}}/Pages/Reference/TexFormat/R32.html)|A single channel of data, with 32 bits per-pixel! This basically treats each pixel as a generic float, so you can do all sorts of strange and interesting things with this.|
|[TexFormat]({{site.url}}/Pages/Reference/TexFormat.html) [R8]({{site.url}}/Pages/Reference/TexFormat/R8.html)|A single channel of data, with 8 bits per-pixel! This can be great when you're only using one channel, and want to reduce memory usage. Values in the shader are always 0.0-1.0.|
|[TexFormat]({{site.url}}/Pages/Reference/TexFormat.html) [Rgba128]({{site.url}}/Pages/Reference/TexFormat/Rgba128.html)|Red/Green/Blue/Transparency data channels at 32 bits per-channel! Basically 4 floats per color, which is bonkers expensive. Don't use this unless you know -exactly- what you're doing.|
|[TexFormat]({{site.url}}/Pages/Reference/TexFormat.html) [Rgba32]({{site.url}}/Pages/Reference/TexFormat/Rgba32.html)|Red/Green/Blue/Transparency data channels, at 8 bits per-channel in sRGB color space. This is what you'll want most of the time you're dealing with color images! Matches well with the Color32 struct! If you're storing normals, rough/metal, or anything else, use Rgba32Linear.|
|[TexFormat]({{site.url}}/Pages/Reference/TexFormat.html) [Rgba32Linear]({{site.url}}/Pages/Reference/TexFormat/Rgba32Linear.html)|Red/Green/Blue/Transparency data channels, at 8 bits per-channel in linear color space. This is what you'll want most of the time you're dealing with color data! Matches well with the Color32 struct.|
|[TexFormat]({{site.url}}/Pages/Reference/TexFormat.html) [Rgba64]({{site.url}}/Pages/Reference/TexFormat/Rgba64.html)|Red/Green/Blue/Transparency data channels, at 16 bits per-channel! This is not common, but you might encounter it with raw photos, or HDR images.|


