---
layout: default
title: Default.MaterialPBR
description: The default Physically Based Rendering material! This is used by StereoKit anytime a mesh or model has metallic or roughness properties, or needs to look more realistic. Its shader may change based on system performance characteristics, so it can be great to copy this one when creating your own materials! Or if you want to override StereoKit's default PBR behavior, here's where you do it! Note that the shader used by default here is much more costly than Default.Material.
---
# [Default]({{site.url}}/Pages/Reference/Default.html).MaterialPBR

<div class='signature' markdown='1'>
static [Material]({{site.url}}/Pages/Reference/Material.html) MaterialPBR{ get set }
</div>

## Description
The default Physically Based Rendering material! This is
used by StereoKit anytime a mesh or model has metallic or
roughness properties, or needs to look more realistic. Its shader
may change based on system performance characteristics, so it can
be great to copy this one when creating your own materials! Or if
you want to override StereoKit's default PBR behavior, here's
where you do it! Note that the shader used by default here is
much more costly than Default.Material.


## Examples

Occlusion (R), Roughness (G), and Metal (B) are stored
respectively in the R, G and B channels of their texture.
Occlusion can be separated out into a different texture as per
the GLTF spec, so you do need to assign it separately from the
Metal texture.
```csharp
matPBR = Default.MaterialPBR.Copy();
matPBR[MatParamName.DiffuseTex  ] = Tex.FromFile("metal_plate_diff.jpg");
matPBR[MatParamName.MetalTex    ] = Tex.FromFile("metal_plate_metal.jpg", false);
matPBR[MatParamName.OcclusionTex] = Tex.FromFile("metal_plate_metal.jpg", false);
```
![PBR material example]({{site.screen_url}}/MaterialPBR.jpg)

