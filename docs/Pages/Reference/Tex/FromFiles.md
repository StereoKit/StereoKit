---
layout: default
title: Tex.FromFiles
description: Loads an array of image files directly into a single array texture! Array textures are often useful for shader effects, layering, material merging, weird stuff, and will generally need a specific shader to support it. Supported formats are. jpg, png, tga, bmp, psd, gif, hdr, pic. Asset Id will be the hash of all the filenames merged consecutively.
---
# [Tex]({{site.url}}/Pages/Reference/Tex.html).FromFiles

<div class='signature' markdown='1'>
```csharp
static Tex FromFiles(String[] files, bool sRGBData, int priority)
```
Loads an array of image files directly into a single
array texture! Array textures are often useful for shader
effects, layering, material merging, weird stuff, and will
generally need a specific shader to support it. Supported formats
are: jpg, png, tga, bmp, psd, gif, hdr, pic. Asset Id will be the
hash of all the filenames merged consecutively.
</div>

|  |  |
|--|--|
|String[] files|Absolute filenames, or a filenames relative             to the assets folder. Supports jpg, png, tga, bmp, psd, gif, hdr,             pic|
|bool sRGBData|Is this image color data in sRGB format,             or is it normal/metal/rough/data that's not for direct display?             sRGB colors get converted to linear color space on the graphics             card, so getting this right can have a big impact on visuals.|
|int priority|The priority sort order for this asset in             the async loading system. Lower values mean loading sooner.|
|RETURNS: [Tex]({{site.url}}/Pages/Reference/Tex.html)|A Tex asset from the given files, or null if it failed to load.|




