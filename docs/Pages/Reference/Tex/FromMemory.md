---
layout: default
title: Tex.FromMemory
description: Loads an image file stored in memory directly into a texture! Supported formats are. jpg, png, tga, bmp, psd, gif, hdr, pic. Asset Id will be the same as the filename.
---
# [Tex]({{site.url}}/Pages/Reference/Tex.html).FromMemory

<div class='signature' markdown='1'>
```csharp
static Tex FromMemory(Byte[]& imageFileData, bool sRGBData, int priority)
```
Loads an image file stored in memory directly into a
texture! Supported formats are: jpg, png, tga, bmp, psd, gif,
hdr, pic. Asset Id will be the same as the filename.
</div>

|  |  |
|--|--|
|Byte[]& imageFileData|The binary data of an image file,             this is NOT a raw RGB color array!|
|bool sRGBData|Is this image color data in sRGB format,             or is it normal/metal/rough/data that's not for direct display?             sRGB colors get converted to linear color space on the graphics             card, so getting this right can have a big impact on visuals.|
|int priority|The priority sort order for this asset in             the async loading system. Lower values mean loading sooner.|
|RETURNS: [Tex]({{site.url}}/Pages/Reference/Tex.html)|A Tex asset from the given file, or null if it failed to load.|




