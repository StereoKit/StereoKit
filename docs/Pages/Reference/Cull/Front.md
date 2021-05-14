---
layout: default
title: Cull.Front
description: Discard if the front of the triangle face is pointing towards the camera. This is opposite the default behavior.
---
# [Cull]({{site.url}}/Pages/Reference/Cull.html).Front

## Description
Discard if the front of the triangle face is pointing
towards the camera. This is opposite the default behavior.


## Examples

Here's setting FaceCull to Front, which is the opposite of the
default behavior. On a sphere, this is a little hard to see, but
you might notice here that the lighting is for the back side of
the sphere!
```csharp
matCull = Default.Material.Copy();
matCull.FaceCull = Cull.Front;
```
![FaceCull material example]({{site.screen_url}}/MaterialCull.jpg)

