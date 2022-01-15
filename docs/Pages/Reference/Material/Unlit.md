---
layout: default
title: Material.Unlit
description: The default unlit material! This is used by StereoKit any time a mesh or model needs to be rendered with an unlit surface. Its shader may change based on system performance characteristics, so it can be great to copy this one when creating your own materials! Or if you want to override StereoKit's default unlit behavior, here's where you do it!
---
# [Material]({{site.url}}/Pages/Reference/Material.html).Unlit

<div class='signature' markdown='1'>
static [Material]({{site.url}}/Pages/Reference/Material.html) Unlit{ get }
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
matUnlit = Material.Unlit.Copy();
matUnlit[MatParamName.DiffuseTex] = Tex.FromFile("floor.png");
```
![Unlit material example]({{site.screen_url}}/MaterialUnlit.jpg)

