---
layout: default
title: Tex.FromEquirectangular
description: Creates a cubemap texture from a single equirectangular image! You know, the ones that look like an unwrapped globe with the poles all streetched out. It uses some fancy shaders and texture blitting to create 6 faces from the equirectangular image.
---
# [Tex]({{site.url}}/Pages/Reference/Tex.html).FromEquirectangular

<div class='signature' markdown='1'>
static [Tex]({{site.url}}/Pages/Reference/Tex.html) FromEquirectangular(string equirectangularCubemap)
</div>

|  |  |
|--|--|
|string equirectangularCubemap|Filename of the equirectangular image.|
|RETURNS: [Tex]({{site.url}}/Pages/Reference/Tex.html)|A Cubemap texture asset!|

Creates a cubemap texture from a single equirectangular image! You know, the ones that
look like an unwrapped globe with the poles all streetched out. It uses some fancy shaders
and texture blitting to create 6 faces from the equirectangular image.



