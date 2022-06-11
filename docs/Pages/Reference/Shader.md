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
|[Shader]({{site.url}}/Pages/Reference/Shader.html) [Default]({{site.url}}/Pages/Reference/Shader/Default.html)|This is a fast, general purpose shader. It uses a texture for 'diffuse', a 'color' property for tinting the material, and a 'tex_scale' for scaling the UV coordinates. For lighting, it just uses a lookup from the current cubemap.|
|[Shader]({{site.url}}/Pages/Reference/Shader.html) [PBR]({{site.url}}/Pages/Reference/Shader/PBR.html)|A physically based shader.|
|[Shader]({{site.url}}/Pages/Reference/Shader.html) [PBRClip]({{site.url}}/Pages/Reference/Shader/PBRClip.html)|Same as ShaderPBR, but with a discard clip for transparency.|
|[Shader]({{site.url}}/Pages/Reference/Shader.html) [UI]({{site.url}}/Pages/Reference/Shader/UI.html)|A shader for UI or interactable elements, this'll be the same as the Shader, but with an additional finger 'shadow' and distance circle effect that helps indicate finger distance from the surface of the object.|
|[Shader]({{site.url}}/Pages/Reference/Shader.html) [UIBox]({{site.url}}/Pages/Reference/Shader/UIBox.html)|A shader for indicating interaction volumes! It renders a border around the edges of the UV coordinates that will 'grow' on proximity to the user's finger. It will discard pixels outside of that border, but will also show the finger shadow. This is meant to be an opaque shader, so it works well for depth LSR.  This shader works best on cube-like meshes where each face has UV coordinates from 0-1.  Shader Parameters: ```color                - color border_size          - meters border_size_grow     - meters border_affect_radius - meters```|
|[Shader]({{site.url}}/Pages/Reference/Shader.html) [Unlit]({{site.url}}/Pages/Reference/Shader/Unlit.html)|Sometimes lighting just gets in the way! This is an extremely simple and fast shader that uses a 'diffuse' texture and a 'color' tint property to draw a model without any lighting at all!|
|[Shader]({{site.url}}/Pages/Reference/Shader.html) [UnlitClip]({{site.url}}/Pages/Reference/Shader/UnlitClip.html)|Sometimes lighting just gets in the way! This is an extremely simple and fast shader that uses a 'diffuse' texture and a 'color' tint property to draw a model without any lighting at all! This shader will also discard pixels with an alpha of zero.|

## Static Methods

|  |  |
|--|--|
|[Find]({{site.url}}/Pages/Reference/Shader/Find.html)|Looks for a Shader asset that's already loaded, matching the given id! Unless the id has been set manually, the id will be the same as the shader's name provided in the metadata.|
|[FromFile]({{site.url}}/Pages/Reference/Shader/FromFile.html)|Loads a shader from a precompiled StereoKit Shader (.sks) file! HLSL files can be compiled using the skshaderc tool included in the NuGet package. This should be taken care of by MsBuild automatically, but you may need to ensure your HLSL file is a <SKShader /> item type in the .csproj for this to work. You can also compile with the command line app manually if you're compiling/distributing a shader some other way!|
|[FromMemory]({{site.url}}/Pages/Reference/Shader/FromMemory.html)|Creates a shader asset from a precompiled StereoKit Shader file stored as bytes!|
