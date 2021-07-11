---
layout: default
title: Material.Wireframe
description: Should this material draw only the edges/wires of the mesh? This can be useful for debugging, and even some kinds of visualization work. Note that this may not work on some mobile OpenGL systems like Quest.
---
# [Material]({{site.url}}/Pages/Reference/Material.html).Wireframe

<div class='signature' markdown='1'>
bool Wireframe{ get set }
</div>

## Description
Should this material draw only the edges/wires of the
mesh? This can be useful for debugging, and even some kinds of
visualization work. Note that this may not work on some mobile
OpenGL systems like Quest.


## Examples

Here's creating a simple wireframe material!
```csharp
matWireframe = Default.Material.Copy();
matWireframe.Wireframe = true;
```
Which looks like this:
![Wireframe material example]({{site.screen_url}}/MaterialWireframe.jpg)

