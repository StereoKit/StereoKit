---
layout: default
title: Renderer.Blit
description: Renders a Material onto a rendertarget texture! StereoKit uses a 4 vert quad stretched over the surface of the texture, and renders the material onto it to the texture.
---
# [Renderer]({{site.url}}/Pages/Reference/Renderer.html).Blit

<div class='signature' markdown='1'>
static void Blit([Tex]({{site.url}}/Pages/Reference/Tex.html) toRendertarget, [Material]({{site.url}}/Pages/Reference/Material.html) material)
</div>

|  |  |
|--|--|
|[Tex]({{site.url}}/Pages/Reference/Tex.html) toRendertarget|A texture that's been set up as a render target!|
|[Material]({{site.url}}/Pages/Reference/Material.html) material|This material is rendered onto the texture! Set it up like you would             if you were applying it to a plane, or quad mesh.|

Renders a Material onto a rendertarget texture! StereoKit uses a 4 vert quad stretched
over the surface of the texture, and renders the material onto it to the texture.



