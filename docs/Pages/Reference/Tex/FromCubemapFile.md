---
layout: default
title: Tex.FromCubemapFile
description: Creates a cubemap texture from 6 different image files! If you have a single equirectangular image, use Tex.FromEquirectangular  instead. Asset Id will be the first filename.
---
# [Tex]({{site.url}}/Pages/Reference/Tex.html).FromCubemapFile

<div class='signature' markdown='1'>
```csharp
static Tex FromCubemapFile(String[] cubeFaceFiles_xxyyzz, bool sRGBData, int priority)
```
Creates a cubemap texture from 6 different image files!
If you have a single equirectangular image, use
Tex.FromEquirectangular  instead. Asset Id will be the first
filename.
</div>

|  |  |
|--|--|
|String[] cubeFaceFiles_xxyyzz|6 image filenames, in order of             +X, -X, +Y, -Y, +Z, -Z.|
|bool sRGBData|Is this image color data in sRGB format,             or is it normal/metal/rough/data that's not for direct display?             sRGB colors get converted to linear color space on the graphics             card, so getting this right can have a big impact on visuals.|
|int priority|The priority sort order for this asset in             the async loading system. Lower values mean loading sooner.|
|RETURNS: [Tex]({{site.url}}/Pages/Reference/Tex.html)|A Tex asset from the given files, or null if any failed to load.|

<div class='signature' markdown='1'>
```csharp
static Tex FromCubemapFile(String[] cubeFaceFiles_xxyyzz, SphericalHarmonics& lightingInfo, bool sRGBData, int priority)
```
Creates a cubemap texture from 6 different image files!
If you have a single equirectangular image, use
Tex.FromEquirectangular instead. Asset Id will be the first
filename.
</div>

|  |  |
|--|--|
|String[] cubeFaceFiles_xxyyzz|6 image filenames, in order of             +X, -X, +Y, -Y, +Z, -Z.|
|SphericalHarmonics& lightingInfo|An out value that represents the             lighting information scraped from the cubemap! This can then be             passed to `Renderer.SkyLight`.|
|bool sRGBData|Is this image color data in sRGB format,             or is it normal/metal/rough/data that's not for direct display?             sRGB colors get converted to linear color space on the graphics             card, so getting this right can have a big impact on visuals.|
|int priority|The priority sort order for this asset in             the async loading system. Lower values mean loading sooner.|
|RETURNS: [Tex]({{site.url}}/Pages/Reference/Tex.html)|A Tex asset from the given files, or null if any failed to load.|




