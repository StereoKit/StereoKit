---
layout: default
title: Default.MaterialUI
description: The material used by the UI! By default, it uses a shader that creates a 'finger shadow' that shows how close the finger is to the UI.
---
# [Default]({{site.url}}/Pages/Reference/Default.html).MaterialUI

<div class='signature' markdown='1'>
static [Material]({{site.url}}/Pages/Reference/Material.html) MaterialUI{ get set }
</div>

## Description
The material used by the UI! By default, it uses a shader
that creates a 'finger shadow' that shows how close the finger is
to the UI.


## Examples

This Material is basically the same as Default.Material, except it
also adds some glow to the surface near the user's fingers. It
works best on flat surfaces, and in StereoKit's design language,
can be used to indicate that something is interactive.
```csharp
matUI = Default.MaterialUI.Copy();
```
And here's what it looks like:
![UI Material example]({{site.screen_url}}/MaterialUI.jpg)

