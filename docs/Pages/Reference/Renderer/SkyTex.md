---
layout: default
title: Renderer.SkyTex
description: Set a cubemap skybox texture for rendering a background! This is only visible on Opaque displays, since transparent displays have the real world behind them already! StereoKit has a a default procedurally generated skybox. You can load one with Tex.FromEquirectangular, Tex.GenCubemap. If you're trying to affect the lighting, see Renderer.SkyLight.
---
# [Renderer]({{site.url}}/Pages/Reference/Renderer.html).SkyTex

<div class='signature' markdown='1'>
static [Tex]({{site.url}}/Pages/Reference/Tex.html) SkyTex{ get set }
</div>

## Description
Set a cubemap skybox texture for rendering a background! This is only visible on Opaque
displays, since transparent displays have the real world behind them already! StereoKit has a
a default procedurally generated skybox. You can load one with `Tex.FromEquirectangular`,
`Tex.GenCubemap`. If you're trying to affect the lighting, see `Renderer.SkyLight`.


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

