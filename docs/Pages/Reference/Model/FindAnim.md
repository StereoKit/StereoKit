---
layout: default
title: Model.FindAnim
description: Searches the list of animations for the first one matching the given name.
---
# [Model]({{site.url}}/Pages/Reference/Model.html).FindAnim

<div class='signature' markdown='1'>
```csharp
Anim FindAnim(string name)
```
Searches the list of animations for the first one matching
the given name.
</div>

|  |  |
|--|--|
|string name|Case sensitive name of the animation.|
|RETURNS: Anim|A link to the animation, or null if none is found.|





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

