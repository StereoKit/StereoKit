---
layout: default
title: MatParamName
description: A better way to access standard shader parameter names, instead of using just strings! If you have your own custom parameters, you can still access them via the string methods, but this is checked and verified by the compiler!
---
# enum MatParamName

A better way to access standard shader parameter names,
instead of using just strings! If you have your own custom
parameters, you can still access them via the string methods, but
this is checked and verified by the compiler!

## Enum Values

|  |  |
|--|--|
|ClipCutoff|In clip shaders, this is the cutoff value below which pixels are discarded. Typically, the diffuse/albedo's alpha component is sampled for comparison here.  This represents the float param 'cutoff'.|
|ColorTint|A per-material color tint, behavior could vary from shader to shader, but often this is just multiplied against the diffuse texture right at the start.  This represents the Color param 'color'.|
|DiffuseTex|The primary color texture for the shader! Diffuse, Albedo, 'The Texture', or whatever you want to call it, this is usually the base color that the shader works with.  This represents the texture param 'diffuse'.|
|EmissionFactor|A multiplier for emission values sampled from the emission texture. The default emission texture in SK shaders is white, and the default value for this parameter is 0,0,0,0.  This represents the Color param 'emission_factor'.|
|EmissionTex|This texture is unaffected by lighting, and is frequently just added in on top of the material's final color! Tends to look really glowy.  This represents the texture param 'emission'.|
|MetallicAmount|For physically based shader, this is a multiplier to scale the metallic properties of the material.  This represents the float param 'metallic'.|
|MetalTex|For physically based shaders, metal is a texture that encodes metallic and roughness data into the 'B' and 'G' channels, respectively.  This represents the texture param 'metal'.|
|NormalTex|The 'normal map' texture for the material! This texture contains information about the direction of the material's surface, which is used to calculate lighting, and make surfaces look like they have more detail than they actually do. Normals are in Tangent Coordinate Space, and the RGB values map to XYZ values.  This represents the texture param 'normal'.|
|OcclusionTex|Used by physically based shaders, this can be used for baked ambient occlusion lighting, or to remove specular reflections from areas that are surrounded by geometry that would likely block reflections.  This represents the texture param 'occlusion'.|
|RoughnessAmount|For physically based shader, this is a multiplier to scale the roughness properties of the material.  This represents the float param 'roughness'.|
|TexScale|Not necessarily present in all shaders, this multiplies the UV coordinates of the mesh, so that the texture will repeat. This is great for tiling textures!  This represents the float param 'tex_scale'.|

## Examples

### Material parameter access
Material does have an array operator overload for setting
shader parameters really quickly! You can do this with strings
representing shader parameter names, or use the MatParamName
enum for compile safety.
```csharp
exampleMaterial[MatParamName.DiffuseTex] = gridTex;
exampleMaterial[MatParamName.TexScale  ] = 2.0f;
```

