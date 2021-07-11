---
layout: default
title: Default.MaterialUIBox
description: A material for indicating interaction volumes! It renders a border around the edges of the UV coordinates that will 'grow' on proximity to the user's finger. It will discard pixels outside of that border, but will also show the finger shadow. This is meant to be an opaque material, so it works well for depth LSR.  This material works best on cube-like meshes where each face has UV coordinates from 0-1.  Shader Parameters. color                - color border_size          - meters border_size_grow     - meters border_affect_radius - meters
---
# [Default]({{site.url}}/Pages/Reference/Default.html).MaterialUIBox

<div class='signature' markdown='1'>
static [Material]({{site.url}}/Pages/Reference/Material.html) MaterialUIBox{ get set }
</div>

## Description
A material for indicating interaction volumes! It
renders a border around the edges of the UV coordinates that will
'grow' on proximity to the user's finger. It will discard pixels
outside of that border, but will also show the finger shadow.
This is meant to be an opaque material, so it works well for
depth LSR.

This material works best on cube-like meshes where each face has
UV coordinates from 0-1.

Shader Parameters:
```color                - color
border_size          - meters
border_size_grow     - meters
border_affect_radius - meters```


## Examples

The UI Box material has 3 parameters to control how the box wires
are rendered. The initial size in meters is 'border_size', and
can grow by 'border_size_grow' meters based on distance to the
user's hand. That distance can be configured via the
'border_affect_radius' property of the shader, which is also in
meters.
```csharp
matUIBox = Default.MaterialUIBox.Copy();
matUIBox["border_size"]          = 0.005f;
matUIBox["border_size_grow"]     = 0.01f;
matUIBox["border_affect_radius"] = 0.2f;
```
![UI box material example]({{site.screen_url}}/MaterialUIBox.jpg)

