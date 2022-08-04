---
layout: default
title: Tex.GenCubemap
description: Generates a cubemap texture from a gradient and a direction! These are entirely suitable for skyboxes, which you can set via Renderer.SkyTex.
---
# [Tex]({{site.url}}/Pages/Reference/Tex.html).GenCubemap

<div class='signature' markdown='1'>
```csharp
static Tex GenCubemap(Gradient gradient, Vec3 gradientDirection, int resolution)
```
Generates a cubemap texture from a gradient and a
direction! These are entirely suitable for skyboxes, which you
can set via Renderer.SkyTex.
</div>

|  |  |
|--|--|
|[Gradient]({{site.url}}/Pages/Reference/Gradient.html) gradient|A color gradient the generator will sample             from! This looks at the 0-1 range of the gradient.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) gradientDirection|This vector points to where the             'top' of the color gradient will go. Conversely, the 'bottom' of             the gradient will be opposite, and it'll blend along that axis.|
|int resolution|The square size in pixels of each cubemap             face! This generally doesn't need to be large, unless you have a             really complicated gradient.|
|RETURNS: [Tex]({{site.url}}/Pages/Reference/Tex.html)|A procedurally generated cubemap texture!|

<div class='signature' markdown='1'>
```csharp
static Tex GenCubemap(Gradient gradient, SphericalHarmonics& lightingInfo, Vec3 gradientDirection, int resolution)
```
Generates a cubemap texture from a gradient and a
direction! These are entirely suitable for skyboxes, which you
can set via `Renderer.SkyTex`.
</div>

|  |  |
|--|--|
|[Gradient]({{site.url}}/Pages/Reference/Gradient.html) gradient|A color gradient the generator will sample             from! This looks at the 0-1 range of the gradient.|
|SphericalHarmonics& lightingInfo|An out value that represents the             lighting information scraped from the cubemap! This can then be             passed to `Renderer.SkyLight`.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) gradientDirection|This vector points to where the             'top' of the color gradient will go. Conversely, the 'bottom' of             the gradient will be opposite, and it'll blend along that axis.|
|int resolution|The square size in pixels of each cubemap             face! This generally doesn't need to be large, unless you have a             really complicated gradient.|
|RETURNS: [Tex]({{site.url}}/Pages/Reference/Tex.html)|A procedurally generated cubemap texture!|

<div class='signature' markdown='1'>
```csharp
static Tex GenCubemap(SphericalHarmonics& lighting, int resolution, float lightSpotSizePct, float lightSpotIntensity)
```
Creates a cubemap from SphericalHarmonics lookups! These
are entirely suitable for skyboxes, which you can set via
`Renderer.SkyTex`.
</div>

|  |  |
|--|--|
|SphericalHarmonics& lighting|Lighting information stored in a             SphericalHarmonics.|
|int resolution|The square size in pixels of each cubemap             face! This generally doesn't need to be large, as             SphericalHarmonics typically contain pretty low frequency             information.|
|float lightSpotSizePct|The size of the glowing spot added             in the primary light direction. You can kinda think of the unit             as a percentage of the cubemap face's size, but it's technically             a Chebyshev distance from the light's point on a 2m cube.|
|float lightSpotIntensity|The glowing spot's color is the             primary light direction's color, but multiplied by this value.             Since this method generates a 128bpp texture, this is not clamped             between 0-1, so feel free to go nuts here! Remember that              reflections will often cut down some reflection intensity.|
|RETURNS: [Tex]({{site.url}}/Pages/Reference/Tex.html)|A procedurally generated cubemap texture!|




