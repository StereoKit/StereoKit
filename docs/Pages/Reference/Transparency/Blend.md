---
layout: default
title: Transparency.Blend
description: This will blend with the pixels behind it. This is transparent! It doesn't write to the z-buffer, and it's slower than opaque materials.
---
# [Transparency]({{site.url}}/Pages/Reference/Transparency.html).Blend

<div class='signature' markdown='1'>
static [Transparency]({{site.url}}/Pages/Reference/Transparency.html) Blend
</div>

## Description
This will blend with the pixels behind it. This is
transparent! It doesn't write to the z-buffer, and it's slower
than opaque materials.


## Examples

### Alpha Blending
Here's an example material with an alpha blend transparency.
Transparent materials typically don't write to the depth buffer,
but this may vary from case to case. Here we're setting the alpha
through the material's Tint value, but the diffuse texture's
alpha and the instance render color's alpha may also play a part
in the final alpha value.
```csharp
matAlphaBlend = Default.Material.Copy();
matAlphaBlend.Transparency = Transparency.Blend;
matAlphaBlend.DepthWrite   = false;
matAlphaBlend[MatParamName.ColorTint] = new Color(1, 1, 1, 0.75f);
```
![Alpha blend example]({{site.screen_url}}/MaterialAlphaBlend.jpg)

