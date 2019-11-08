---
layout: default
title: Shader
description: A shader is a piece of code that runs on the GPU, and determines how model data gets transformed into pixels on screen! It's more likely that you'll work more directly with Materials, which shaders are a subset of.  With this particular class, you can mostly just look at it. It doesn't do a whole lot. Maybe you can swap out the shader code or something sometimes!
---
# Shader

## Description
A shader is a piece of code that runs on the GPU, and determines
how model data gets transformed into pixels on screen! It's more likely
that you'll work more directly with Materials, which shaders are a subset of.

With this particular class, you can mostly just look at it. It doesn't do
a whole lot. Maybe you can swap out the shader code or something sometimes!



## Methods

|  |  |
|--|--|
|[Find]({{site.url}}/Pages/Reference/Shader/Find.html)|Finds a shader asset with a given id.|
|[FromHLSL]({{site.url}}/Pages/Reference/Shader/FromHLSL.html)|Creates a shader from a piece of HLSL code! Shader stuff like this may change in the future, since HLSL may not be all that portable. Also, before compiling the shader code, StereoKit hashes the contents, and looks to see if it has that shader cached. If so, it'll just load that instead of compiling it again.|


