---
layout: default
title: Material.UIBox
description: A material for indicating interaction volumes! It renders a border around the edges of the UV coordinates that will 'grow' on proximity to the user's finger. It will discard pixels outside of that border, but will also show the finger shadow. This is meant to be an opaque material, so it works well for depth LSR.  This material works best on cube-like meshes where each face has UV coordinates from 0-1.  Shader Parameters. color                - color border_size          - meters border_size_grow     - meters border_affect_radius - meters
---
# [Material]({{site.url}}/Pages/Reference/Material.html).UIBox

<div class='signature' markdown='1'>
static [Material]({{site.url}}/Pages/Reference/Material.html) UIBox{ get }
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
matUIBox = Material.UIBox.Copy();
matUIBox["border_size"]          = 0.005f;
matUIBox["border_size_grow"]     = 0.01f;
matUIBox["border_affect_radius"] = 0.2f;
```
![UI box material example]({{site.screen_url}}/MaterialUIBox.jpg)
### An Interactive Model

![A grabbable GLTF Model using UI.Handle]({{site.screen_url}}/HandleBox.jpg)

If you want to grab a Model and move it around, then you can use a
`UI.Handle` to do it! Here's an example of loading a GLTF from file,
and using its information to create a Handle and a UI 'cage' box that
indicates an interactive element.

```csharp
Model model      = Model.FromFile("DamagedHelmet.gltf");
Pose  handlePose = new Pose(0,0,0, Quat.Identity);
float scale      = .15f;

public void Step() {
	UI.HandleBegin("Model Handle", ref handlePose, model.Bounds*scale);

	model.Draw(Matrix.S(scale));
	Mesh.Cube.Draw(Material.UIBox, Matrix.TS(model.Bounds.center*scale, model.Bounds.dimensions*scale));

	UI.HandleEnd();
}
```

