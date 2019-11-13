---
layout: default
title: Shader.Find
description: Looks for a Material asset that's already loaded, matching the given id! Unless the id has been set manually, the id will be the same as the filename provided for loading the shader.
---
# [Shader]({{site.url}}/Pages/Reference/Shader.html).Find

<div class='signature' markdown='1'>
static [Shader]({{site.url}}/Pages/Reference/Shader.html) Find(string shaderId)
</div>

|  |  |
|--|--|
|string shaderId|For shaders loaded from file, this'll be the file name!|
|RETURNS: [Shader]({{site.url}}/Pages/Reference/Shader.html)|Link to a shader asset!|

Looks for a Material asset that's already loaded, matching the given id! Unless
the id has been set manually, the id will be the same as the filename provided for
loading the shader.



