---
layout: default
title: Default.ShaderFont
description: A shader for text! Right now, this will render a font atlas texture, and perform alpha-testing for transparency, and super-sampling for better readability. It also flips normals of the back-face of the surface, so  backfaces get lit properly when backface culling is turned off, as it is by default for text.
---
# [Default]({{site.url}}/Pages/Reference/Default.html).ShaderFont

<div class='signature' markdown='1'>
static [Shader]({{site.url}}/Pages/Reference/Shader.html) ShaderFont{ get set }
</div>

## Description
A shader for text! Right now, this will render a font
atlas texture, and perform alpha-testing for transparency, and
super-sampling for better readability. It also flips normals of
the back-face of the surface, so  backfaces get lit properly when
backface culling is turned off, as it is by default for text.

