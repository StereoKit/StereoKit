---
layout: default
title: Default.Shader
description: This is a fast, general purpose shader. It uses a texture for 'diffuse', a 'color' property for tinting the material, and a 'tex_scale' for scaling the UV coordinates. For lighting, it just uses a lookup from the current cubemap.
---
# [Default]({{site.url}}/Pages/Reference/Default.html).Shader

<div class='signature' markdown='1'>
static [Shader]({{site.url}}/Pages/Reference/Shader.html) Shader{ get set }
</div>

## Description
This is a fast, general purpose shader. It uses a
texture for 'diffuse', a 'color' property for tinting the
material, and a 'tex_scale' for scaling the UV coordinates. For
lighting, it just uses a lookup from the current cubemap.

