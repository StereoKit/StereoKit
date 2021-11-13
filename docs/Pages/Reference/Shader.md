---
layout: default
title: Shader
description: A shader is a piece of code that runs on the GPU, and determines how model data gets transformed into pixels on screen! It's more likely that you'll work more directly with Materials, which shaders are a subset of.  With this particular class, you can mostly just look at it. It doesn't do a whole lot. Maybe you can swap out the shader code or something sometimes!
---
# class Shader

A shader is a piece of code that runs on the GPU, and
determines how model data gets transformed into pixels on screen!
It's more likely that you'll work more directly with Materials, which
shaders are a subset of.

With this particular class, you can mostly just look at it. It doesn't
do a whole lot. Maybe you can swap out the shader code or something
sometimes!


## Instance Fields and Properties

|  |  |
|--|--|
|string [Name]({{site.url}}/Pages/Reference/Shader/Name.html)|The name of the shader, provided in the shader file itself. Not the filename or id.|



## Static Fields and Properties

|  |  |
|--|--|
|[Shader]({{site.url}}/Pages/Reference/Shader.html) [Default]({{site.url}}/Pages/Reference/Shader/Default.html)|See `Default.Shader`|
|[Shader]({{site.url}}/Pages/Reference/Shader.html) [PBR]({{site.url}}/Pages/Reference/Shader/PBR.html)|See `Default.ShaderPbr`|
|[Shader]({{site.url}}/Pages/Reference/Shader.html) [PBRClip]({{site.url}}/Pages/Reference/Shader/PBRClip.html)|See `Default.ShaderPbrClip`|
|[Shader]({{site.url}}/Pages/Reference/Shader.html) [UI]({{site.url}}/Pages/Reference/Shader/UI.html)|See `Default.ShaderUI`|
|[Shader]({{site.url}}/Pages/Reference/Shader.html) [UIBox]({{site.url}}/Pages/Reference/Shader/UIBox.html)|See `Default.ShaderUIBox`|
|[Shader]({{site.url}}/Pages/Reference/Shader.html) [Unlit]({{site.url}}/Pages/Reference/Shader/Unlit.html)|See `Default.ShaderUnlit`|
|[Shader]({{site.url}}/Pages/Reference/Shader.html) [UnlitClip]({{site.url}}/Pages/Reference/Shader/UnlitClip.html)|See `Default.ShaderUnlitClip`|


## Static Methods

|  |  |
|--|--|
|[Find]({{site.url}}/Pages/Reference/Shader/Find.html)|Looks for a Shader asset that's already loaded, matching the given id! Unless the id has been set manually, the id will be the same as the shader's name provided in the metadata.|
|[FromFile]({{site.url}}/Pages/Reference/Shader/FromFile.html)|Loads a shader from a precompiled StereoKit Shader (.sks) file! HLSL files can be compiled using the skshaderc tool included in the NuGet package. This should be taken care of by MsBuild automatically, but you may need to ensure your HLSL file is a <SKShader /> item type in the .csproj for this to work. You can also compile with the command line app manually if you're compiling/distributing a shader some other way!|
|[FromMemory]({{site.url}}/Pages/Reference/Shader/FromMemory.html)|Creates a shader asset from a precompiled StereoKit Shader file stored as bytes!|

