---
layout: default
title: Mesh.Bounds
description: This is a bounding box that encapsulates the Mesh! It's used for collision, visibility testing, UI layout, and probably other things. While it's normally calculated from the mesh vertices, you can also override this to suit your needs.
---
# [Mesh]({{site.url}}/Pages/Reference/Mesh.html).Bounds

<div class='signature' markdown='1'>
[Bounds]({{site.url}}/Pages/Reference/Bounds.html) Bounds{ get set }
</div>

## Description
This is a bounding box that encapsulates the Mesh! It's
used for collision, visibility testing, UI layout, and probably
other things. While it's normally calculated from the mesh vertices,
you can also override this to suit your needs.


## Examples

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

