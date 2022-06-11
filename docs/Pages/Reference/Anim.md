---
layout: default
title: Anim
description: A link to a Model's animation! You can use this to get some basic information about the animation, or store it for reference. This maintains a link to the Model asset, and will keep it alive as long as this object lives.
---
# class Anim

A link to a Model's animation! You can use this to get some
basic information about the animation, or store it for reference. This
maintains a link to the Model asset, and will keep it alive as long as
this object lives.

## Instance Fields and Properties

|  |  |
|--|--|
|float [Duration]({{site.url}}/Pages/Reference/Anim/Duration.html)|The duration of the animation at normal playback speed, in seconds.|
|string [Name]({{site.url}}/Pages/Reference/Anim/Name.html)|The name of the animation as provided by the original asset.|

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

