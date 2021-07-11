---
layout: default
title: Default.MaterialUnlit
description: The default unlit material! This is used by StereoKit any time a mesh or model needs to be rendered with an unlit surface. Its shader may change based on system performance characteristics, so it can be great to copy this one when creating your own materials! Or if you want to override StereoKit's default unlit behavior, here's where you do it!
---
# [Default]({{site.url}}/Pages/Reference/Default.html).MaterialUnlit

<div class='signature' markdown='1'>
static [Material]({{site.url}}/Pages/Reference/Material.html) MaterialUnlit{ get set }
</div>

## Description
The default unlit material! This is used by StereoKit
any time a mesh or model needs to be rendered with an unlit
surface. Its shader may change based on system performance
characteristics, so it can be great to copy this one when
creating your own materials! Or if you want to override
StereoKit's default unlit behavior, here's where you do it!


## Examples

```csharp
matUnlit = Default.MaterialUnlit.Copy();
matUnlit[MatParamName.DiffuseTex] = Tex.FromFile("floor.png");
```
![Unlit material example]({{site.screen_url}}/MaterialUnlit.jpg)

