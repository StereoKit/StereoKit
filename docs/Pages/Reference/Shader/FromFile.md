---
layout: default
title: Shader.FromFile
description: Loads and compiles a shader from an hlsl, or precompiled StereoKit Shader file! After loading an hlsl file, StereoKit will hash it, and check to see if it has changed since the last time it cached a compiled version. If there is no cache for the hash, it'll compile it, and save the compiled shader to a cache folder in the asset path! The Id will be the shader's internal name.
---
# [Shader]({{site.url}}/Pages/Reference/Shader.html).FromFile

<div class='signature' markdown='1'>
static [Shader]({{site.url}}/Pages/Reference/Shader.html) FromFile(string file)
</div>

|  |  |
|--|--|
|string file|Path to a StereoKit Shader file, or hlsl code.              This gets prefixed with the asset path in StereoKitApp.settings.|
|RETURNS: [Shader]({{site.url}}/Pages/Reference/Shader.html)|A shader from the given file, or null if it failed to load/compile.|

Loads and compiles a shader from an hlsl, or precompiled
StereoKit Shader file! After loading an hlsl file, StereoKit will
hash it, and check to see if it has changed since the last time
it cached a compiled version. If there is no cache for the hash,
it'll compile it, and save the compiled shader to a cache folder
in the asset path! The Id will be the shader's internal name.



