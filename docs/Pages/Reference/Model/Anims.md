---
layout: default
title: Model.Anims
description: An enumerable collection of animations attached to this Model. You can do Linq stuff with it, foreach it, or just treat it like a List or array!
---
# [Model]({{site.url}}/Pages/Reference/Model.html).Anims

<div class='signature' markdown='1'>
[ModelAnimCollection]({{site.url}}/Pages/Reference/ModelAnimCollection.html) Anims{ get }
</div>

## Description
An enumerable collection of animations attached to this
Model. You can do Linq stuff with it, foreach it, or just treat it
like a List or array!


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

