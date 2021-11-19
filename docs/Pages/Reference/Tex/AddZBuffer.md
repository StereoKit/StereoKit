---
layout: default
title: Tex.AddZBuffer
description: Only applicable if this texture is a rendertarget! This creates and attaches a zbuffer surface to the texture for use when rendering to it.
---
# [Tex]({{site.url}}/Pages/Reference/Tex.html).AddZBuffer

<div class='signature' markdown='1'>
```csharp
void AddZBuffer(TexFormat depthFormat)
```
Only applicable if this texture is a rendertarget!
This creates and attaches a zbuffer surface to the texture for
use when rendering to it.
</div>

|  |  |
|--|--|
|[TexFormat]({{site.url}}/Pages/Reference/TexFormat.html) depthFormat|The format of the depth texture, must             be a depth format type!|




