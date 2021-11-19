---
layout: default
title: Model.AnimMode
description: The playback mode of the active animation.
---
# [Model]({{site.url}}/Pages/Reference/Model.html).AnimMode

<div class='signature' markdown='1'>
[AnimMode]({{site.url}}/Pages/Reference/AnimMode.html) AnimMode{ get }
</div>

## Description
The playback mode of the active animation.


## Examples

### Animation progress bar
A really simple progress bar visualization for the Model's active
animation.

![Model with progress bar]({{site.screen_url}}/AnimProgress.jpg)
```csharp
model.Draw(Matrix.Identity);

Hierarchy.Push(Matrix.T(0.5f, 1, -.25f));

// This is a pair of green lines that show the current progress through
// the animation.
float progress = model.AnimCompletion;
Lines.Add(V.XY0(0, 0), V.XY0(-progress, 0),  new Color(0,1,0,1.0f), 2*U.cm);
Lines.Add(V.XY0(-progress, 0), V.XY0(-1, 0), new Color(0,1,0,0.2f), 2*U.cm);

// These are some labels for the progress bar that tell us more about
// the active animation.
Text.Add($"{model.ActiveAnim.Name} : {model.AnimMode}", Matrix.TS(0, -2*U.cm, 0, 3),        TextAlign.TopLeft);
Text.Add($"{model.AnimTime:F1}s",                       Matrix.TS(-progress, 2*U.cm, 0, 3), TextAlign.BottomCenter);

Hierarchy.Pop();
```

