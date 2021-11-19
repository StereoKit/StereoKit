---
layout: default
title: Color.ToHSV
description: Converts the gamma space color to a Hue/Saturation/Value format! Does not consider transparency when calculating the result.
---
# [Color]({{site.url}}/Pages/Reference/Color.html).ToHSV

<div class='signature' markdown='1'>
```csharp
Vec3 ToHSV()
```
Converts the gamma space color to a Hue/Saturation/Value
format! Does not consider transparency when calculating the
result.
</div>

|  |  |
|--|--|
|RETURNS: [Vec3]({{site.url}}/Pages/Reference/Vec3.html)|Hue, Saturation, and Value, stored in x, y, and z respectively. All values are between 0-1.|





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

