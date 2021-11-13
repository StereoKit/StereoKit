---
layout: default
title: Tex.FromFile
description: Loads an image file directly into a texture! Supported formats are. jpg, png, tga, bmp, psd, gif, hdr, pic. Asset Id will be the same as the filename.
---
# [Tex]({{site.url}}/Pages/Reference/Tex.html).FromFile

<div class='signature' markdown='1'>
```csharp
static Tex FromFile(string file, bool sRGBData)
```
Loads an image file directly into a texture! Supported
formats are: jpg, png, tga, bmp, psd, gif, hdr, pic. Asset Id
will be the same as the filename.
</div>

|  |  |
|--|--|
|string file|An absolute filename, or a filename relative              to the assets folder. Supports jpg, png, tga, bmp, psd, gif, hdr,             pic|
|bool sRGBData|Is this image color data in sRGB format,             or is it normal/metal/rough/data that's not for direct display?             sRGB colors get converted to linear color space on the graphics             card, so getting this right can have a big impact on visuals.|
|RETURNS: [Tex]({{site.url}}/Pages/Reference/Tex.html)|A Tex asset from the given file, or null if it failed to load.|




