---
layout: default
title: Material
description: A Material describes the surface of anything drawn on the graphics card! It is typically composed of a Shader, and shader properties like colors, textures, transparency info, etc.  Items drawn with the same Material can be batched together into a single, fast operation on the graphics card, so re-using materials can be extremely beneficial for performance!
---
# Material

## Description
A Material describes the surface of anything drawn on the graphics
card! It is typically composed of a Shader, and shader properties like colors,
textures, transparency info, etc.

Items drawn with the same Material can be batched together into a single, fast
operation on the graphics card, so re-using materials can be extremely beneficial
for performance!



## Methods

|  |  |
|--|--|
|[Copy]({{site.url}}/Pages/Reference/Material/Copy.html)|Creates a new Material asset with the same shader and properties! Draw calls with the new Material will not batch together with this one.|
|[Copy]({{site.url}}/Pages/Reference/Material/Copy.html)|Creates a new Material asset with the same shader and properties! Draw calls with the new Material will not batch togethre with the source Material.|
|[SetColor]({{site.url}}/Pages/Reference/Material/SetColor.html)|Sets a shader parameter with the given name to the provided value. If no parameter is found, nothing happens, and the value is not set!|
|[SetColor]({{site.url}}/Pages/Reference/Material/SetColor.html)|Sets a shader parameter with the given name to the provided value. If no parameter is found, nothing happens, and the value is not set!|
|[SetFloat]({{site.url}}/Pages/Reference/Material/SetFloat.html)|Sets a shader parameter with the given name to the provided value. If no parameter is found, nothing happens, and the value is not set!|
|[SetMatrix]({{site.url}}/Pages/Reference/Material/SetMatrix.html)|Sets a shader parameter with the given name to the provided value. If no parameter is found, nothing happens, and the value is not set!|
|[SetTexture]({{site.url}}/Pages/Reference/Material/SetTexture.html)|Sets a shader parameter with the given name to the provided value. If no parameter is found, nothing happens, and the value is not set!|
|[SetVector]({{site.url}}/Pages/Reference/Material/SetVector.html)|Sets a shader parameter with the given name to the provided value. If no parameter is found, nothing happens, and the value is not set!|


