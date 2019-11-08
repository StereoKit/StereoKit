---
layout: default
title: Shader.FromHLSL
description: Creates a shader from a piece of HLSL code! Shader stuff like this may change in the future, since HLSL may not be all that portable. Also, before compiling the shader code, StereoKit hashes the contents, and looks to see if it has that shader cached. If so, it'll just load that instead of compiling it again.
---
# [Shader]({{site.url}}/Pages/Reference/Shader.html).FromHLSL
<div class='signature' markdown='1'>
static [Shader]({{site.url}}/Pages/Reference/Shader.html) FromHLSL(string hlsl)
</div>
Creates a shader from a piece of HLSL code! Shader stuff like this may
change in the future, since HLSL may not be all that portable. Also, before
compiling the shader code, StereoKit hashes the contents, and looks to see if
it has that shader cached. If so, it'll just load that instead of compiling it
again.

|  |  |
|--|--|
|string hlsl|A vertex and pixel shader written in HLSL, check the shader             guides for more on this later!|
|RETURNS: [Shader]({{site.url}}/Pages/Reference/Shader.html)||



