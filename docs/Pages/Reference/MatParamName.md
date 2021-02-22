---
layout: default
title: MatParamName
description: An better way to access standard shader paramter names, instead of using just strings! If you have your own custom parameters, you can still access them via the string methods, but this is checked and verified by the compiler!
---
# MatParamName

An better way to access standard shader paramter names,
instead of using just strings! If you have your own custom
parameters, you can still access them via the string methods, but
this is checked and verified by the compiler!




## Static Fields and Properties

|  |  |
|--|--|
|[MatParamName]({{site.url}}/Pages/Reference/MatParamName.html) [ColorTint]({{site.url}}/Pages/Reference/MatParamName/ColorTint.html)|A per-material color tint, behavior could vary from shader to shader, but often this is just multiplied against the diffuse texture right at the start.  This represents the Color param 'color'.|
|[MatParamName]({{site.url}}/Pages/Reference/MatParamName.html) [DiffuseTex]({{site.url}}/Pages/Reference/MatParamName/DiffuseTex.html)|The primary color texture for the shader! Diffuse, Albedo, 'The Texture', or whatever you want to call it, this is usually the base color that the shader works with.  This represents the texture param 'diffuse'.|
|[MatParamName]({{site.url}}/Pages/Reference/MatParamName.html) [EmissionTex]({{site.url}}/Pages/Reference/MatParamName/EmissionTex.html)|This texture is unaffected by lighting, and is frequently just added in on top of the material's final color! Tends to look really glowy.  This represents the texture param 'emission'.|
|[MatParamName]({{site.url}}/Pages/Reference/MatParamName.html) [MetallicAmount]({{site.url}}/Pages/Reference/MatParamName/MetallicAmount.html)|For physically based shader, this is a multiplier to scale the metallic properties of the material.  This represents the float param 'metallic'.|
|[MatParamName]({{site.url}}/Pages/Reference/MatParamName.html) [MetalTex]({{site.url}}/Pages/Reference/MatParamName/MetalTex.html)|For physically based shaders, metal is a texture that encodes metallic and roughness data into the 'B' and 'G' channels, respectively.  This represents the texture param 'metal'.|
|[MatParamName]({{site.url}}/Pages/Reference/MatParamName.html) [NormalTex]({{site.url}}/Pages/Reference/MatParamName/NormalTex.html)|The 'normal map' texture for the material! This texture contains information about the direction of the material's surface, which is used to calculate lighting, and make surfaces look like they have more detail than they actually do. Normals are in Tangent Coordinate Space, and the RGB values map to XYZ values.  This represents the texture param 'normal'.|
|[MatParamName]({{site.url}}/Pages/Reference/MatParamName.html) [OcclusionTex]({{site.url}}/Pages/Reference/MatParamName/OcclusionTex.html)|Used by physically based shaders, this can be used for baked ambient occlusion lighting, or to remove specular reflections from areas that are surrounded by geometry that would likely block reflections.  This represents the texture param 'occlusion'.|
|[MatParamName]({{site.url}}/Pages/Reference/MatParamName.html) [RoughnessAmount]({{site.url}}/Pages/Reference/MatParamName/RoughnessAmount.html)|For physically based shader, this is a multiplier to scale the roughness properties of the material.  This represents the float param 'roughness'.|
|[MatParamName]({{site.url}}/Pages/Reference/MatParamName.html) [TexScale]({{site.url}}/Pages/Reference/MatParamName/TexScale.html)|Not necessarily present in all shaders, this multiplies the UV coordinates of the mesh, so that the texture will repeat. This is great for tiling textures!  This represents the float param 'tex_scale'.|



## Examples

## Material parameter access
Material does have an array operator overload for setting
shader parameters really quickly! You can do this with strings
representing shader parameter names, or use the MatParamName
enum for compile safety.
```csharp
exampleMaterial[MatParamName.DiffuseTex] = gridTex;
exampleMaterial[MatParamName.TexScale  ] = 2;
```

