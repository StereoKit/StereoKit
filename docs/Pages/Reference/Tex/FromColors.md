---
layout: default
title: Tex.FromColors
description: Creates a texture and sets the texture's pixels using a color array! This will be an image of type TexType.Image, and a format of TexFormat.Rgba32 or TexFormat.Rgba32Linear depending on the value of the sRGBData parameter.
---
# [Tex]({{site.url}}/Pages/Reference/Tex.html).FromColors

<div class='signature' markdown='1'>
```csharp
static Tex FromColors(Color32[]& colors, int width, int height, bool sRGBData)
```
Creates a texture and sets the texture's pixels using a
color array! This will be an image of type `TexType.Image`, and
a format of `TexFormat.Rgba32` or `TexFormat.Rgba32Linear`
depending on the value of the sRGBData parameter.
</div>

|  |  |
|--|--|
|Color32[]& colors|An array of 32 bit colors, should be a             length of `width*height`.|
|int width|Width in pixels of the texture. Powers of two             are generally best!|
|int height|Height in pixels of the texture. Powers of             two are generally best!|
|bool sRGBData|Is this image color data in sRGB format,             or is it normal/metal/rough/data that's not for direct display?             sRGB colors get converted to linear color space on the graphics             card, so getting this right can have a big impact on visuals.|
|RETURNS: [Tex]({{site.url}}/Pages/Reference/Tex.html)|A Tex asset with TexType.Image and TexFormat.Rgba32 from the given array of colors.|

<div class='signature' markdown='1'>
```csharp
static Tex FromColors(Color[]& colors, int width, int height, bool sRGBData)
```
Creates a texture and sets the texture's pixels using a
color array! Color values are converted to 32 bit colors, so this
means a memory allocation and conversion. Prefer the Color32
overload for performance, or create an empty Texture and use
SetColors for more flexibility. This will be an image of type
`TexType.Image`, and a format of `TexFormat.Rgba32` or
`TexFormat.Rgba32Linear` depending on the value of the sRGBData
parameter.
</div>

|  |  |
|--|--|
|Color[]& colors|An array of 128 bit colors, should be a             length of `width*height`.|
|int width|Width in pixels of the texture. Powers of two             are generally best!|
|int height|Height in pixels of the texture. Powers of             two are generally best!|
|bool sRGBData|Is this image color data in sRGB format,             or is it normal/metal/rough/data that's not for direct display?             sRGB colors get converted to linear color space on the graphics             card, so getting this right can have a big impact on visuals.|
|RETURNS: [Tex]({{site.url}}/Pages/Reference/Tex.html)|A Tex asset with TexType.Image and TexFormat.Rgba32 from the given array of colors.|




