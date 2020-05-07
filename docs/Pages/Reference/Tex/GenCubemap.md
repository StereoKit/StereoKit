---
layout: default
title: Tex.GenCubemap
description: Generates a cubemap texture from a gradient and a direction! These are entirely suitable for skyboxes, which you can set via Renderer.SkyTex.
---
# [Tex]({{site.url}}/Pages/Reference/Tex.html).GenCubemap

<div class='signature' markdown='1'>
static [Tex]({{site.url}}/Pages/Reference/Tex.html) GenCubemap([Gradient]({{site.url}}/Pages/Reference/Gradient.html) gradient, [Vec3]({{site.url}}/Pages/Reference/Vec3.html) gradientDirection, int resolution)
</div>

|  |  |
|--|--|
|[Gradient]({{site.url}}/Pages/Reference/Gradient.html) gradient|A color gradient the generator will sample             from! This looks at the 0-1 range of the gradient.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) gradientDirection|This vector points to where the              'top' of the color gradient will go. Conversely, the 'bottom' of              the gradient will be opposite, and it'll blend along that axis.|
|int resolution|The square size in pixels of each cubemap              face! This generally doesn't need to be large, unless you have a              really complicated gradient.|
|RETURNS: [Tex]({{site.url}}/Pages/Reference/Tex.html)|A procedurally generated cubemap texture!|

Generates a cubemap texture from a gradient and a
direction! These are entirely suitable for skyboxes, which you
can set via Renderer.SkyTex.
<div class='signature' markdown='1'>
static [Tex]({{site.url}}/Pages/Reference/Tex.html) GenCubemap([Gradient]({{site.url}}/Pages/Reference/Gradient.html) gradient, SphericalHarmonics& lightingInfo, [Vec3]({{site.url}}/Pages/Reference/Vec3.html) gradientDirection, int resolution)
</div>

|  |  |
|--|--|
|[Gradient]({{site.url}}/Pages/Reference/Gradient.html) gradient|A color gradient the generator will sample              from! This looks at the 0-1 range of the gradient.|
|SphericalHarmonics& lightingInfo|An out value that represents the              lighting information scraped from the cubemap! This can then be              passed to `Renderer.SkyLight`.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) gradientDirection|This vector points to where the              'top' of the color gradient will go. Conversely, the 'bottom' of              the gradient will be opposite, and it'll blend along that axis.|
|int resolution|The square size in pixels of each cubemap             face! This generally doesn't need to be large, unless you have a             really complicated gradient.|
|RETURNS: [Tex]({{site.url}}/Pages/Reference/Tex.html)|A procedurally generated cubemap texture!|

Generates a cubemap texture from a gradient and a
direction! These are entirely suitable for skyboxes, which you
can set via `Renderer.SkyTex`.
<div class='signature' markdown='1'>
static [Tex]({{site.url}}/Pages/Reference/Tex.html) GenCubemap(SphericalHarmonics& lighting, int resolution)
</div>

|  |  |
|--|--|
|SphericalHarmonics& lighting|Lighting information stored in a              SphericalHarmonics.|
|int resolution|The square size in pixels of each cubemap              face! This generally doesn't need to be large, as SphericalHarmonics              typically contain pretty low frequency information.|
|RETURNS: [Tex]({{site.url}}/Pages/Reference/Tex.html)|A procedurally generated cubemap texture!|

Creates a cubemap from SphericalHarmonics lookups! These
are entirely suitable for skyboxes, which you can set via
`Renderer.SkyTex`.



