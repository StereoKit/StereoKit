---
layout: default
title: Material
description: A Material describes the surface of anything drawn on the graphics card! It is typically composed of a Shader, and shader properties like colors, textures, transparency info, etc.  Items drawn with the same Material can be batched together into a single, fast operation on the graphics card, so re-using materials can be extremely beneficial for performance!
---
# class Material

A Material describes the surface of anything drawn on the
graphics card! It is typically composed of a Shader, and shader
properties like colors, textures, transparency info, etc.

Items drawn with the same Material can be batched together into a
single, fast operation on the graphics card, so re-using materials
can be extremely beneficial for performance!

## Instance Fields and Properties

|  |  |
|--|--|
|[DepthTest]({{site.url}}/Pages/Reference/DepthTest.html) [DepthTest]({{site.url}}/Pages/Reference/Material/DepthTest.html)|How does this material interact with the ZBuffer? Generally DepthTest.Less would be normal behavior: don't draw objects that are occluded. But this can also be used to achieve some interesting effects, like you could use DepthTest.Greater to draw a glow that indicates an object is behind something.|
|bool [DepthWrite]({{site.url}}/Pages/Reference/Material/DepthWrite.html)|Should this material write to the ZBuffer? For opaque objects, this generally should be true. But transparent objects writing to the ZBuffer can be problematic and cause draw order issues. Note that turning this off can mean that this material won't get properly accounted for when the MR system is performing late stage reprojection.  Not writing to the buffer can also be faster! :)|
|[Cull]({{site.url}}/Pages/Reference/Cull.html) [FaceCull]({{site.url}}/Pages/Reference/Material/FaceCull.html)|How should this material cull faces?|
|int [ParamCount]({{site.url}}/Pages/Reference/Material/ParamCount.html)|The number of shader parameters available to this material, includes global shader variables as well as textures.|
|int [QueueOffset]({{site.url}}/Pages/Reference/Material/QueueOffset.html)|This property will force this material to draw earlier or later in the draw queue. Positive values make it draw later, negative makes it earlier. This can be helpful for tweaking performance! If you know an object is always going to be close to the user and likely to obscure lots of objects (like hands), drawing it earlier can mean objects behind it get discarded much faster! Similarly, objects that are far away (skybox!) can be pushed towards the back of the queue, so they're more likely to be discarded early.|
|[Shader]({{site.url}}/Pages/Reference/Shader.html) [Shader]({{site.url}}/Pages/Reference/Material/Shader.html)|Gets a link to the Shader that the Material is currently using, or overrides the Shader this material uses.|
|[Transparency]({{site.url}}/Pages/Reference/Transparency.html) [Transparency]({{site.url}}/Pages/Reference/Material/Transparency.html)|What type of transparency does this Material use? Default is None. Transparency has an impact on performance, and draw order. Check the Transparency enum for details.|
|bool [Wireframe]({{site.url}}/Pages/Reference/Material/Wireframe.html)|Should this material draw only the edges/wires of the mesh? This can be useful for debugging, and even some kinds of visualization work. Note that this may not work on some mobile OpenGL systems like Quest.|

## Instance Methods

|  |  |
|--|--|
|[Material]({{site.url}}/Pages/Reference/Material/Material.html)|Creates a material from a shader, and uses the shader's default settings. Uses an auto-generated id.|
|[Copy]({{site.url}}/Pages/Reference/Material/Copy.html)|Creates a new Material asset with the same shader and properties! Draw calls with the new Material will not batch together with this one.|
|[GetAllParamInfo]({{site.url}}/Pages/Reference/Material/GetAllParamInfo.html)|Gets an enumerable list of all parameter information on the Material. This includes all global shader variables and textures.|
|[GetParamInfo]({{site.url}}/Pages/Reference/Material/GetParamInfo.html)|Gets available shader parameter information for the parameter at the indicated index. Parameters are listed as variables first, then textures.|
|[SetBool]({{site.url}}/Pages/Reference/Material/SetBool.html)|Sets a shader parameter with the given name to the provided value. If no parameter is found, nothing happens, and the value is not set!|
|[SetColor]({{site.url}}/Pages/Reference/Material/SetColor.html)|Sets a shader parameter with the given name to the provided value. If no parameter is found, nothing happens, and the value is not set!|
|[SetData]({{site.url}}/Pages/Reference/Material/SetData.html)|This allows you to set more complex shader data types such as structs. Note the SK doesn't guard against setting data of the wrong size here, so pay extra attention to the size of your data here, and ensure it matched up with the shader!|
|[SetFloat]({{site.url}}/Pages/Reference/Material/SetFloat.html)|Sets a shader parameter with the given name to the provided value. If no parameter is found, nothing happens, and the value is not set!|
|[SetInt]({{site.url}}/Pages/Reference/Material/SetInt.html)|Sets a shader parameter with the given name to the provided value. If no parameter is found, nothing happens, and the value is not set!|
|[SetMatrix]({{site.url}}/Pages/Reference/Material/SetMatrix.html)|Sets a shader parameter with the given name to the provided value. If no parameter is found, nothing happens, and the value is not set!|
|[SetTexture]({{site.url}}/Pages/Reference/Material/SetTexture.html)|Sets a shader parameter with the given name to the provided value. If no parameter is found, nothing happens, and the value is not set!|
|[SetUInt]({{site.url}}/Pages/Reference/Material/SetUInt.html)|Sets a shader parameter with the given name to the provided value. If no parameter is found, nothing happens, and the value is not set!|
|[SetVector]({{site.url}}/Pages/Reference/Material/SetVector.html)|Sets a shader parameter with the given name to the provided value. If no parameter is found, nothing happens, and the value is not set!|

## Static Fields and Properties

|  |  |
|--|--|
|[Material]({{site.url}}/Pages/Reference/Material.html) [Default]({{site.url}}/Pages/Reference/Material/Default.html)|The default material! This is used by many models and meshes rendered from within StereoKit. Its shader is tuned for high performance, and may change based on system performance characteristics, so it can be great to copy this one when creating your own materials! Or if you want to override StereoKit's default material, here's where you do it!|
|[Material]({{site.url}}/Pages/Reference/Material.html) [PBR]({{site.url}}/Pages/Reference/Material/PBR.html)|The default Physically Based Rendering material! This is used by StereoKit anytime a mesh or model has metallic or roughness properties, or needs to look more realistic. Its shader may change based on system performance characteristics, so it can be great to copy this one when creating your own materials! Or if you want to override StereoKit's default PBR behavior, here's where you do it! Note that the shader used by default here is much more costly than Default.Material.|
|[Material]({{site.url}}/Pages/Reference/Material.html) [PBRClip]({{site.url}}/Pages/Reference/Material/PBRClip.html)|Same as MaterialPBR, but it uses a discard clip for transparency.|
|[Material]({{site.url}}/Pages/Reference/Material.html) [UI]({{site.url}}/Pages/Reference/Material/UI.html)|The material used by the UI! By default, it uses a shader that creates a 'finger shadow' that shows how close the finger is to the UI.|
|[Material]({{site.url}}/Pages/Reference/Material.html) [UIBox]({{site.url}}/Pages/Reference/Material/UIBox.html)|A material for indicating interaction volumes! It renders a border around the edges of the UV coordinates that will 'grow' on proximity to the user's finger. It will discard pixels outside of that border, but will also show the finger shadow. This is meant to be an opaque material, so it works well for depth LSR.  This material works best on cube-like meshes where each face has UV coordinates from 0-1.  Shader Parameters: ```color                - color border_size          - meters border_size_grow     - meters border_affect_radius - meters```|
|[Material]({{site.url}}/Pages/Reference/Material.html) [UIQuadrant]({{site.url}}/Pages/Reference/Material/UIQuadrant.html)|The material used by the UI for Quadrant Sized UI elements. See UI.QuadrantSizeMesh for additional details. By default, it uses a shader that creates a 'finger shadow' that shows how close the finger is to the UI.|
|[Material]({{site.url}}/Pages/Reference/Material.html) [Unlit]({{site.url}}/Pages/Reference/Material/Unlit.html)|The default unlit material! This is used by StereoKit any time a mesh or model needs to be rendered with an unlit surface. Its shader may change based on system performance characteristics, so it can be great to copy this one when creating your own materials! Or if you want to override StereoKit's default unlit behavior, here's where you do it!|
|[Material]({{site.url}}/Pages/Reference/Material.html) [UnlitClip]({{site.url}}/Pages/Reference/Material/UnlitClip.html)|The default unlit material with alpha clipping! This is used by StereoKit for unlit content with transparency, where completely transparent pixels are discarded. This means less alpha blending, and fewer visible alpha blending issues! In particular, this is how Sprites are drawn. Its shader may change based on system performance characteristics, so it can be great to copy this one when creating your own materials! Or if you want to override StereoKit's default unlit clipped behavior, here's where you do it!|

## Static Methods

|  |  |
|--|--|
|[Find]({{site.url}}/Pages/Reference/Material/Find.html)|Looks for a Material asset that's already loaded, matching the given id!|

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

