---
layout: default
title: Anim.Duration
description: The duration of the animation at normal playback speed, in seconds.
---
# [Anim]({{site.url}}/Pages/Reference/Anim.html).Duration

<div class='signature' markdown='1'>
float Duration{ get }
</div>

## Description
The duration of the animation at normal playback speed, in
seconds.


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

