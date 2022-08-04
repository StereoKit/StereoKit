---
layout: default
title: UI.HandleBegin
description: This begins a new UI group with its own layout! Much like a window, except with a more flexible handle, and no header. You can draw the handle, but it will have no text on it. The pose value is always relative to the current hierarchy stack. This call will also push the pose transform onto the hierarchy stack, so any objects drawn up to the corresponding UI.HandleEnd() will get transformed by the handle pose. Returns true for every frame the user is grabbing the handle.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).HandleBegin

<div class='signature' markdown='1'>
```csharp
static bool HandleBegin(string id, Pose& pose, Bounds handle, bool drawHandle, UIMove moveType)
```
This begins a new UI group with its own layout! Much
like a window, except with a more flexible handle, and no header.
You can draw the handle, but it will have no text on it.
The pose value is always relative to the current hierarchy stack.
This call will also push the pose transform onto the hierarchy stack, so
any objects drawn up to the corresponding UI.HandleEnd() will get transformed
by the handle pose. Returns true for every frame the user is grabbing the handle.
</div>

|  |  |
|--|--|
|string id|An id for tracking element state. MUST be unique             within current hierarchy.|
|Pose& pose|The pose state for the handle! The user will              be able to grab this handle and move it around. The pose is relative             to the current hierarchy stack.|
|[Bounds]({{site.url}}/Pages/Reference/Bounds.html) handle|Size and location of the handle, relative to              the pose.|
|bool drawHandle|Should this function draw the handle              visual for you, or will you draw that yourself?|
|[UIMove]({{site.url}}/Pages/Reference/UIMove.html) moveType|Describes how the handle will move when              dragged around.|
|RETURNS: bool|Returns true for every frame the user is grabbing the handle.|





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

