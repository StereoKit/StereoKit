---
layout: default
title: SphericalHarmonics.DominantLightDirection
description: Returns the dominant direction of the light represented by this spherical harmonics data. The direction value is normalized.  You can get the color of the light in this direction by using the struct's Sample method. light.Sample(-light.DominantLightDirection).
---
# [SphericalHarmonics]({{site.url}}/Pages/Reference/SphericalHarmonics.html).DominantLightDirection

<div class='signature' markdown='1'>
[Vec3]({{site.url}}/Pages/Reference/Vec3.html) DominantLightDirection{ get }
</div>

## Description
Returns the dominant direction of the light represented
by this spherical harmonics data. The direction value is
normalized.

You can get the color of the light in this direction by using the
struct's Sample method:
`light.Sample(-light.DominantLightDirection)`.

