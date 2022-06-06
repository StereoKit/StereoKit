---
layout: default
title: AnimMode
description: Describes how an animation is played back, and what to do when the animation hits the end.
---
# enum AnimMode

Describes how an animation is played back, and what to do when
the animation hits the end.

## Enum Values

|  |  |
|--|--|
|Loop|If the animation reaches the end, it will always loop back around to the start again.|
|Manual|The animation will not progress on its own, and instead must be driven by providing information to the model's AnimTime or AnimCompletion properties.|
|Once|When the animation reaches the end, it will freeze in-place.|

## Examples

### Loading an animated Model
Here, we're loading a Model that we know has the animations "Idle"
and "Jump". This sample shows some options, but only a single call
to PlayAnim is necessary to start an animation.
```csharp
Model model = Model.FromFile("Cosmonaut.glb");

// You can look at the model's animations:
foreach (Anim anim in model.Anims)
	Log.Info($"Animation: {anim.Name} {anim.Duration}s");

// You can play an animation like this
model.PlayAnim("Jump", AnimMode.Once);

// Or you can find and store the animations in advance
Anim jumpAnim = model.FindAnim("Idle");
if (jumpAnim != null)
	model.PlayAnim(jumpAnim, AnimMode.Loop);
```

