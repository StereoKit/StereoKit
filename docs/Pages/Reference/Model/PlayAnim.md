---
layout: default
title: Model.PlayAnim
description: Searches for an animation with the given name, and if it's found, sets it up as the active animation and begins playing it with the animation mode.
---
# [Model]({{site.url}}/Pages/Reference/Model.html).PlayAnim

<div class='signature' markdown='1'>
```csharp
void PlayAnim(string animationName, AnimMode mode)
```
Searches for an animation with the given name, and if it's
found, sets it up as the active animation and begins playing it
with the animation mode.
</div>

|  |  |
|--|--|
|string animationName|Case sensitive name of the animation.|
|[AnimMode]({{site.url}}/Pages/Reference/AnimMode.html) mode|The mode with which to play the animation.|

<div class='signature' markdown='1'>
```csharp
void PlayAnim(Anim animation, AnimMode mode)
```
Sets the animation up as the active animation, and begins
playing it with the animation mode.
</div>

|  |  |
|--|--|
|Anim animation|The new active animation.|
|[AnimMode]({{site.url}}/Pages/Reference/AnimMode.html) mode|The mode with which to play the animation.|





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

