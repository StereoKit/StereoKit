---
layout: default
title: Material
description: A Material describes the surface of anything drawn on the graphics card! It is typically composed of a Shader, and shader properties like colors, textures, transparency info, etc.  Items drawn with the same Material can be batched together into a single, fast operation on the graphics card, so re-using materials can be extremely beneficial for performance!
---
# Material

A Material describes the surface of anything drawn on the graphics
card! It is typically composed of a Shader, and shader properties like colors,
textures, transparency info, etc.

Items drawn with the same Material can be batched together into a single, fast
operation on the graphics card, so re-using materials can be extremely beneficial
for performance!



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
|[Cull]({{site.url}}/Pages/Reference/Cull.html) [FaceCull]({{site.url}}/Pages/Reference/Material/FaceCull.html)|How should this material cull faces?|
|int [ParamCount]({{site.url}}/Pages/Reference/Material/ParamCount.html)|The number of shader parameters available to this material.|
|int [QueueOffset]({{site.url}}/Pages/Reference/Material/QueueOffset.html)|This property will force this material to draw earlier or later in the draw queue. Positive values make it draw later, negative makes it earlier. This can be helpful for tweaking performance! If you know an object is always going to be close to the user and likely to obscure lots of objects (like hands), drawing it earlier can mean objects behind it get discarded much faster! Similarly, objects that are far away (skybox!) can be pushed towards the back of the queue, so they're more likely to be discarded early.|
|[Shader]({{site.url}}/Pages/Reference/Shader.html) [Shader]({{site.url}}/Pages/Reference/Material/Shader.html)|Gets a link to the Shader that the Material is currently using|
|[Transparency]({{site.url}}/Pages/Reference/Transparency.html) [Transparency]({{site.url}}/Pages/Reference/Material/Transparency.html)|What type of transparency does this Material use? Default is None. Transparency has an impact on performance, and draw order. Check the Transparency enum for details.|


## Static Methods

|  |  |
|--|--|
|[Find]({{site.url}}/Pages/Reference/Material/Find.html)|Looks for a Material asset that's already loaded, matching the given id!|

