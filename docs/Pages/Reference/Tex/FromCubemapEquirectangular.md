---
layout: default
title: Tex.FromCubemapEquirectangular
description: Creates a cubemap texture from a single equirectangular image! You know, the ones that look like an unwrapped globe with the poles all streetched out. It uses some fancy shaders and texture blitting to create 6 faces from the equirectangular image.
---
# [Tex]({{site.url}}/Pages/Reference/Tex.html).FromCubemapEquirectangular

<div class='signature' markdown='1'>
static [Tex]({{site.url}}/Pages/Reference/Tex.html) FromCubemapEquirectangular(string equirectangularCubemap, bool sRGBData)
</div>

|  |  |
|--|--|
|string equirectangularCubemap|Filename of the equirectangular             image.|
|bool sRGBData|Is this image color data in sRGB format,              or is it normal/metal/rough/data that's not for direct display?             sRGB colors get converted to linear color space on the graphics             card, so getting this right can have a big impact on visuals.|
|RETURNS: [Tex]({{site.url}}/Pages/Reference/Tex.html)|A Cubemap texture asset!|

Creates a cubemap texture from a single equirectangular
image! You know, the ones that look like an unwrapped globe with
the poles all streetched out. It uses some fancy shaders and
texture blitting to create 6 faces from the equirectangular
image.
<div class='signature' markdown='1'>
static [Tex]({{site.url}}/Pages/Reference/Tex.html) FromCubemapEquirectangular(string equirectangularCubemap, SphericalHarmonics& lightingInfo, bool sRGBData)
</div>

|  |  |
|--|--|
|string equirectangularCubemap|Filename of the equirectangular              image.|
|SphericalHarmonics& lightingInfo|An out value that represents the              lighting information scraped from the cubemap! This can then be             passed to `Renderer.SkyLight`.|
|bool sRGBData|Is this image color data in sRGB format,              or is it normal/metal/rough/data that's not for direct display?              sRGB colors get converted to linear color space on the graphics             card, so getting this right can have a big impact on visuals.|
|RETURNS: [Tex]({{site.url}}/Pages/Reference/Tex.html)|A Cubemap texture asset!|

Creates a cubemap texture from a single equirectangular
image! You know, the ones that look like an unwrapped globe with
the poles all streetched out. It uses some fancy shaders and texture
blitting to create 6 faces from the equirectangular image.



