---
layout: default
title: Renderer.SkyLight
description: Sets the lighting information for the scene! You can build one through SphericalHarmonics.FromLights, or grab one from Tex.FromEquirectangular or Tex.GenCubemap
---
# [Renderer]({{site.url}}/Pages/Reference/Renderer.html).SkyLight

<div class='signature' markdown='1'>
static [SphericalHarmonics]({{site.url}}/Pages/Reference/SphericalHarmonics.html) SkyLight{ get set }
</div>

## Description
Sets the lighting information for the scene! You can
build one through `SphericalHarmonics.FromLights`, or grab one
from `Tex.FromEquirectangular` or `Tex.GenCubemap`


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

