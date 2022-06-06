---
layout: default
title: Tex.FromCubemapEquirectangular
description: Creates a cubemap texture from a single equirectangular image! You know, the ones that look like an unwrapped globe with the poles all stretched out. It uses some fancy shaders and texture blitting to create 6 faces from the equirectangular image.
---
# [Tex]({{site.url}}/Pages/Reference/Tex.html).FromCubemapEquirectangular

<div class='signature' markdown='1'>
```csharp
static Tex FromCubemapEquirectangular(string equirectangularCubemap, bool sRGBData, int loadPriority)
```
Creates a cubemap texture from a single equirectangular
image! You know, the ones that look like an unwrapped globe with
the poles all stretched out. It uses some fancy shaders and
texture blitting to create 6 faces from the equirectangular
image.
</div>

|  |  |
|--|--|
|string equirectangularCubemap|Filename of the             equirectangular image.|
|bool sRGBData|Is this image color data in sRGB format,             or is it normal/metal/rough/data that's not for direct display?             sRGB colors get converted to linear color space on the graphics             card, so getting this right can have a big impact on visuals.|
|int loadPriority|The priority sort order for this asset             in the async loading system. Lower values mean loading sooner.|
|RETURNS: [Tex]({{site.url}}/Pages/Reference/Tex.html)|A Cubemap texture asset!|

<div class='signature' markdown='1'>
```csharp
static Tex FromCubemapEquirectangular(string equirectangularCubemap, SphericalHarmonics& lightingInfo, bool sRGBData, int loadPriority)
```
Creates a cubemap texture from a single equirectangular
image! You know, the ones that look like an unwrapped globe with
the poles all stretched out. It uses some fancy shaders and
texture blitting to create 6 faces from the equirectangular image.
</div>

|  |  |
|--|--|
|string equirectangularCubemap|Filename of the             equirectangular image.|
|SphericalHarmonics& lightingInfo|An out value that represents the             lighting information scraped from the cubemap! This can then be             passed to `Renderer.SkyLight`.|
|bool sRGBData|Is this image color data in sRGB format,             or is it normal/metal/rough/data that's not for direct display?             sRGB colors get converted to linear color space on the graphics             card, so getting this right can have a big impact on visuals.|
|int loadPriority|The priority sort order for this asset             in the async loading system. Lower values mean loading sooner.|
|RETURNS: [Tex]({{site.url}}/Pages/Reference/Tex.html)|A Cubemap texture asset!|





## Examples

### Setting lighting to an equirect cubemap
Changing the environment's lighting based on an image is a really
great way to instantly get a particular feel to your scene! A neat
place to find compatible equirectangular images for this is
[Poly Haven](https://polyhaven.com/hdris)
```csharp
Renderer.SkyTex   = Tex.FromCubemapEquirectangular("old_depot.hdr", out SphericalHarmonics lighting);
Renderer.SkyLight = lighting;
```
And here's what it looks like applied to the default Material!
![Default Material example]({{site.screen_url}}/MaterialDefault.jpg)

