---
layout: default
title: Shader
description: A shader is a piece of code that runs on the GPU, and determines how model data gets transformed into pixels on screen! It's more likely that you'll work more directly with Materials, which shaders are a subset of.  With this particular class, you can mostly just look at it. It doesn't do a whole lot. Maybe you can swap out the shader code or something sometimes!
---
# Shader

A shader is a piece of code that runs on the GPU, and
determines how model data gets transformed into pixels on screen!
It's more likely that you'll work more directly with Materials, which
shaders are a subset of.

With this particular class, you can mostly just look at it. It doesn't
do a whole lot. Maybe you can swap out the shader code or something
sometimes!




## Static Fields and Properties

|  |  |
|--|--|
|string [Name]({{site.url}}/Pages/Reference/Shader/Name.html)|The name of the shader, provided in the shader file itself. Not the filename or id.|


## Static Methods

|  |  |
|--|--|
|[Compile]({{site.url}}/Pages/Reference/Shader/Compile.html)|This will compile an HLSL shader into a StereoKit binary storage format. StereoKit does not need to be initialized for this method to work! The binary format includes data about shader parameters and textures, as well as pre-compiled shaders for different platforms. Currently only supports HLSL, but can support more. The results can be passed into Shader.FromMemory to get a final shader asset, or it can be saved to file! Note: If the shader uses file relative #includes, they will not work without providing the filename of the hlsl. See the other overload for this method!|
|[Find]({{site.url}}/Pages/Reference/Shader/Find.html)|Looks for a Shader asset that's already loaded, matching the given id! Unless the id has been set manually, the id will be the same as the shader's name provided in the metadata.|
|[FromFile]({{site.url}}/Pages/Reference/Shader/FromFile.html)|Loads and compiles a shader from an hlsl, or precompiled StereoKit Shader file! After loading an hlsl file, StereoKit will hash it, and check to see if it has changed since the last time it cached a compiled version. If there is no cache for the hash, it'll compile it, and save the compiled shader to a cache folder in the asset path! The Id will be the shader's internal name.|
|[FromHLSL]({{site.url}}/Pages/Reference/Shader/FromHLSL.html)|Creates a shader from a piece of HLSL code! Shader stuff like this may change in the future, since HLSL may not be all that portable. Also, before compiling the shader code, StereoKit hashes the contents, and looks to see if it has that shader cached. If so, it'll just load that instead of compiling it again. The Id will be the shader's internal name.|
|[FromMemory]({{site.url}}/Pages/Reference/Shader/FromMemory.html)|Creates a shader asset from a precompiled StereoKit Shader file stored as bytes! If you don't have a precompiled shader file, you can make one with the Shader.Compile method. The Id will be the shader's internal name.|

