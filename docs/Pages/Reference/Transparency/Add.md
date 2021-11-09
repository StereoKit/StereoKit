---
layout: default
title: Transparency.Add
description: This will straight up add the pixel color to the color buffer! This usually looks -really- glowy, so it makes for good particles or lighting effects.
---
# [Transparency]({{site.url}}/Pages/Reference/Transparency.html).Add

<div class='signature' markdown='1'>
static [Transparency]({{site.url}}/Pages/Reference/Transparency.html) Add
</div>

## Description
This will straight up add the pixel color to the color
buffer! This usually looks -really- glowy, so it makes for good
particles or lighting effects.


## Examples

### Additive Transparency
Here's an example material with additive transparency.
Transparent materials typically don't write to the depth buffer,
but this may vary from case to case. Note that the material's
alpha does not play any role in additive transparency! Instead,
you could make the material's tint darker.
```csharp
matAlphaAdd = Default.Material.Copy();
matAlphaAdd.Transparency = Transparency.Add;
matAlphaAdd.DepthWrite   = false;
```
![Additive transparency example]({{site.screen_url}}/MaterialAlphaAdd.jpg)

