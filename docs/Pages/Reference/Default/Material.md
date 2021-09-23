---
layout: default
title: Default.Material
description: The default material! This is used by many models and meshes rendered from within StereoKit. Its shader is tuned for high performance, and may change based on system performance characteristics, so it can be great to copy this one when creating your own materials! Or if you want to override StereoKit's default material, here's where you do it!
---
# [Default]({{site.url}}/Pages/Reference/Default.html).Material

<div class='signature' markdown='1'>
static [Material]({{site.url}}/Pages/Reference/Material.html) Material{ get set }
</div>

## Description
The default material! This is used by many models and
meshes rendered from within StereoKit. Its shader is tuned for
high performance, and may change based on system performance
characteristics, so it can be great to copy this one when
creating your own materials! Or if you want to override
StereoKit's default material, here's where you do it!


## Examples

If you want to modify the default material, it's recommended to
copy it first!
```csharp
matDefault = Default.Material.Copy();
```
And here's what it looks like:
![Default Material example]({{site.screen_url}}/MaterialDefault.jpg)

