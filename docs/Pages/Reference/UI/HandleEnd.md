---
layout: default
title: UI.HandleEnd
description: Finishes a handle! Must be called after UI.HandleBegin() and all elements have been drawn. Pops the pose transform pushed by UI.HandleBegin() from the hierarchy stack.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).HandleEnd

<div class='signature' markdown='1'>
```csharp
static void HandleEnd()
```
Finishes a handle! Must be called after UI.HandleBegin()
and all elements have been drawn. Pops the pose transform pushed
by UI.HandleBegin() from the hierarchy stack.
</div>





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

