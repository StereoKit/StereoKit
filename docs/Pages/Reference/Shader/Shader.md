---
layout: default
title: Shader.Shader
description: Loads and compiles a shader from an hlsl file! Technically, after loading the file, StereoKit will hash it, and check to see if it has changed since the last time it cached a compiled version. If there is no cache for the hash, it'll compile it, and save the compiled shader to a cache folder in the asset path!
---
# [Shader]({{site.url}}/Pages/Reference/Shader.html).Shader

<div class='signature' markdown='1'>
 Shader(string file)
</div>

|  |  |
|--|--|
|string file|Path to a file with hlsl code in it. This gets prefixed with the asset path             in StereoKitApp.settings.|

Loads and compiles a shader from an hlsl file! Technically, after loading the file,
StereoKit will hash it, and check to see if it has changed since the last time it cached a
compiled version. If there is no cache for the hash, it'll compile it, and save the compiled
shader to a cache folder in the asset path!



