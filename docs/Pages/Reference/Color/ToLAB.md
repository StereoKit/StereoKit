---
layout: default
title: Color.ToLAB
description: Converts the gamma space RGB color to a CIE LAB color space value! Conversion back and forth from LAB space could be somewhat lossy.
---
# [Color]({{site.url}}/Pages/Reference/Color.html).ToLAB

<div class='signature' markdown='1'>
```csharp
Vec3 ToLAB()
```
Converts the gamma space RGB color to a CIE LAB color
space value! Conversion back and forth from LAB space could be
somewhat lossy.
</div>

|  |  |
|--|--|
|RETURNS: [Vec3]({{site.url}}/Pages/Reference/Vec3.html)|An LAB vector where x=L, y=A, z=B.|





## Examples

```csharp
// Desaturating a color can be done quite nicely with the HSV
// functions
Color red      = new Color(1,0,0,1);
Vec3  colorHSV = red.ToHSV();
colorHSV.y *= 0.5f; // Drop saturation by half
Color desaturatedRed = Color.HSV(colorHSV, red.a);

// LAB color space is excellent for modifying perceived 
// brightness, or 'Lightness' of a color.
Color green    = new Color(0,1,0,1);
Vec3  colorLAB = green.ToLAB();
colorLAB.x *= 0.5f; // Drop lightness by half
Color darkGreen = Color.LAB(colorLAB, green.a);
```

