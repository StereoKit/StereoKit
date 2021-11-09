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
|int [ParamCount]({{site.url}}/Pages/Reference/Material/ParamCount.html)|The number of shader parameters available to this material.|
|int [QueueOffset]({{site.url}}/Pages/Reference/Material/QueueOffset.html)|This property will force this material to draw earlier or later in the draw queue. Positive values make it draw later, negative makes it earlier. This can be helpful for tweaking performance! If you know an object is always going to be close to the user and likely to obscure lots of objects (like hands), drawing it earlier can mean objects behind it get discarded much faster! Similarly, objects that are far away (skybox!) can be pushed towards the back of the queue, so they're more likely to be discarded early.|
|[Shader]({{site.url}}/Pages/Reference/Shader.html) [Shader]({{site.url}}/Pages/Reference/Material/Shader.html)|Gets a link to the Shader that the Material is currently using, or overrides the Shader this material uses.|
|[Transparency]({{site.url}}/Pages/Reference/Transparency.html) [Transparency]({{site.url}}/Pages/Reference/Material/Transparency.html)|What type of transparency does this Material use? Default is None. Transparency has an impact on performance, and draw order. Check the Transparency enum for details.|
|bool [Wireframe]({{site.url}}/Pages/Reference/Material/Wireframe.html)|Should this material draw only the edges/wires of the mesh? This can be useful for debugging, and even some kinds of visualization work. Note that this may not work on some mobile OpenGL systems like Quest.|


## Instance Methods

|  |  |
|--|--|
|[Material]({{site.url}}/Pages/Reference/Material/Material.html)|Creates a material from a shader, and uses the shader's default settings. Uses an auto-generated id.|
|[Copy]({{site.url}}/Pages/Reference/Material/Copy.html)|Creates a new Material asset with the same shader and properties! Draw calls with the new Material will not batch together with this one.|
|[SetColor]({{site.url}}/Pages/Reference/Material/SetColor.html)|Sets a shader parameter with the given name to the provided value. If no parameter is found, nothing happens, and the value is not set!|
|[SetFloat]({{site.url}}/Pages/Reference/Material/SetFloat.html)|Sets a shader parameter with the given name to the provided value. If no parameter is found, nothing happens, and the value is not set!|
|[SetMatrix]({{site.url}}/Pages/Reference/Material/SetMatrix.html)|Sets a shader parameter with the given name to the provided value. If no parameter is found, nothing happens, and the value is not set!|
|[SetTexture]({{site.url}}/Pages/Reference/Material/SetTexture.html)|Sets a shader parameter with the given name to the provided value. If no parameter is found, nothing happens, and the value is not set!|
|[SetVector]({{site.url}}/Pages/Reference/Material/SetVector.html)|Sets a shader parameter with the given name to the provided value. If no parameter is found, nothing happens, and the value is not set!|


## Static Fields and Properties

|  |  |
|--|--|
|[Material]({{site.url}}/Pages/Reference/Material.html) [Default]({{site.url}}/Pages/Reference/Material/Default.html)|See `Default.Material`|
|[Material]({{site.url}}/Pages/Reference/Material.html) [PBR]({{site.url}}/Pages/Reference/Material/PBR.html)|See `Default.MaterialPBR`|
|[Material]({{site.url}}/Pages/Reference/Material.html) [PBRClip]({{site.url}}/Pages/Reference/Material/PBRClip.html)|See `Default.MaterialPBRClip`|
|[Material]({{site.url}}/Pages/Reference/Material.html) [UI]({{site.url}}/Pages/Reference/Material/UI.html)|See `Default.MaterialUI`|
|[Material]({{site.url}}/Pages/Reference/Material.html) [UIBox]({{site.url}}/Pages/Reference/Material/UIBox.html)|See `Default.MaterialUIBox`|
|[Material]({{site.url}}/Pages/Reference/Material.html) [Unlit]({{site.url}}/Pages/Reference/Material/Unlit.html)|See `Default.MaterialUnlit`|
|[Material]({{site.url}}/Pages/Reference/Material.html) [UnlitClip]({{site.url}}/Pages/Reference/Material/UnlitClip.html)|See `Default.MaterialUnlitClip`|


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
exampleMaterial[MatParamName.TexScale  ] = 2;
```

