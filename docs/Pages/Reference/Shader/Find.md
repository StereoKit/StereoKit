---
layout: default
title: Shader.Find
description: Looks for a Shader asset that's already loaded, matching the given id! Unless the id has been set manually, the id will be the same as the shader's name provided in the metadata.
---
# [Shader]({{site.url}}/Pages/Reference/Shader.html).Find

<div class='signature' markdown='1'>
```csharp
static Shader Find(string shaderId)
```
Looks for a Shader asset that's already loaded, matching
the given id! Unless the id has been set manually, the id will be
the same as the shader's name provided in the metadata.
</div>

|  |  |
|--|--|
|string shaderId|For shaders loaded from file, this'll be              the shader's metadata name!|
|RETURNS: [Shader]({{site.url}}/Pages/Reference/Shader.html)|Link to a shader asset!|




