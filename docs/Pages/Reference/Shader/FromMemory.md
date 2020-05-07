---
layout: default
title: Shader.FromMemory
description: Creates a shader asset from a precompiled StereoKit Shader file stored as bytes! If you don't have a precompiled shader file, you can make one with the Shader.Compile method. The Id will be the shader's internal name.
---
# [Shader]({{site.url}}/Pages/Reference/Shader.html).FromMemory

<div class='signature' markdown='1'>
static [Shader]({{site.url}}/Pages/Reference/Shader.html) FromMemory(Byte[]& data)
</div>

|  |  |
|--|--|
|Byte[]& data|A precompiled StereoKit Shader file as bytes,             you can get these bytes from Shader.Compile!|
|RETURNS: [Shader]({{site.url}}/Pages/Reference/Shader.html)|A shader from the given data, or null if it failed to load/compile.|

Creates a shader asset from a precompiled StereoKit
Shader file stored as bytes! If you don't have a precompiled
shader file, you can make one with the Shader.Compile method.
The Id will be the shader's internal name.



