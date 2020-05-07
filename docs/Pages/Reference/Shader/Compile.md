---
layout: default
title: Shader.Compile
description: This will compile an HLSL shader into a StereoKit binary storage format. StereoKit does not need to be initialized for this method to work! The binary format includes data about shader parameters and textures, as well as pre-compiled shaders for different platforms. Currently only supports HLSL, but can support more. The results can be passed into Shader.FromMemory to get a final shader asset, or it can be saved to file! Note. If the shader uses file relative #includes, they will not work without providing the filename of the hlsl. See the other overload for this method!
---
# [Shader]({{site.url}}/Pages/Reference/Shader.html).Compile

<div class='signature' markdown='1'>
static Byte[] Compile(string hlsl)
</div>

|  |  |
|--|--|
|string hlsl|HLSL code, including metadata comments.|
|RETURNS: Byte[]|Binary data representing a StereoKit compiled shader data file. This can be passed into Shader.FromMemory to get a final shader asset, or it can be saved to file!|

This will compile an HLSL shader into a StereoKit binary
storage format. StereoKit does not need to be initialized for
this method to work! The binary format includes data about shader
parameters and textures, as well as pre-compiled shaders for
different platforms. Currently only supports HLSL, but can support
more. The results can be passed into Shader.FromMemory to get a
final shader asset, or it can be saved to file!
Note: If the shader uses file relative #includes, they will not
work without providing the filename of the hlsl. See the other
overload for this method!
<div class='signature' markdown='1'>
static Byte[] Compile(string hlsl, string filename)
</div>

|  |  |
|--|--|
|string hlsl|HLSL code, including metadata comments.|
|string filename|The file that this shader code came from!             This is used to find relative #includes in the shader code.|
|RETURNS: Byte[]|Binary data representing a StereoKit compiled shader data file. This can be passed into Shader.FromMemory to get a final shader asset, or it can be saved to file!|

This will compile an HLSL shader into a StereoKit binary
storage format. StereoKit does not need to be initialized for
this method to work! The binary format includes data about shader
parameters and textures, as well as pre-compiled shaders for
different platforms. Currently only supports HLSL, but can support
more. The results can be passed into Shader.FromMemory to get a
final shader asset, or it can be saved to file!



