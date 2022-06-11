---
layout: default
title: TexFormat
description: What type of color information will the texture contain? A good default here is Rgba32.
---
# enum TexFormat

What type of color information will the texture contain? A
good default here is Rgba32.

## Enum Values

|  |  |
|--|--|
|Bgra32|Red/Green/Blue/Transparency data channels, at 8 bits per-channel in linear color space. This is what you'll want most of the time you're dealing with color data! Matches well with the Color32 struct.|
|Bgra32Linear|Red/Green/Blue/Transparency data channels, at 8 bits per-channel in linear color space. This is what you'll want most of the time you're dealing with color data! Matches well with the Color32 struct.|
|Depth16|16 bits of depth is not a lot, but it can be enough if your far clipping plane is pretty close. If you're seeing lots of flickering where two objects overlap, you either need to bring your far clip in, or switch to 32/24 bit depth.|
|Depth32|32 bits of data per depth value! This is pretty detailed, and is excellent for experiences that have a very far view distance.|
|DepthStencil|A depth data format, 24 bits for depth data, and 8 bits to store stencil information! Stencil data can be used for things like clipping effects, deferred rendering, or shadow effects.|
|None|A default zero value for TexFormat! Uninitialized formats will get this value and **** **** up so you know to assign it properly :)|
|R16|A single channel of data, with 16 bits per-pixel! This is a good format for height maps, since it stores a fair bit of information in it. Values in the shader are always 0.0-1.0.|
|R32|A single channel of data, with 32 bits per-pixel! This basically treats each pixel as a generic float, so you can do all sorts of strange and interesting things with this.|
|R8|A single channel of data, with 8 bits per-pixel! This can be great when you're only using one channel, and want to reduce memory usage. Values in the shader are always 0.0-1.0.|
|Rg11b10|Red/Green/Blue/Transparency data channels, at 8 bits per-channel in linear color space. This is what you'll want most of the time you're dealing with color data! Matches well with the Color32 struct.|
|Rgb10a2|Red/Green/Blue/Transparency data channels, at 8 bits per-channel in linear color space. This is what you'll want most of the time you're dealing with color data! Matches well with the Color32 struct.|
|Rgba128|Red/Green/Blue/Transparency data channels at 32 bits per-channel! Basically 4 floats per color, which is bonkers expensive. Don't use this unless you know -exactly- what you're doing.|
|Rgba32|Red/Green/Blue/Transparency data channels, at 8 bits per-channel in sRGB color space. This is what you'll want most of the time you're dealing with color images! Matches well with the Color32 struct! If you're storing normals, rough/metal, or anything else, use Rgba32Linear.|
|Rgba32Linear|Red/Green/Blue/Transparency data channels, at 8 bits per-channel in linear color space. This is what you'll want most of the time you're dealing with color data! Matches well with the Color32 struct.|
|Rgba64|TODO: remove during major version update|
|Rgba64u|16 bits of depth is not a lot, but it can be enough if your far clipping plane is pretty close. If you're seeing lots of flickering where two objects overlap, you either need to bring your far clip in, or switch to 32/24 bit depth.|
