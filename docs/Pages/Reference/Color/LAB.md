---
layout: default
title: Color.LAB
description: Creates a gamma space RGB color from a CIE-L*ab color space. CIE-L*ab is a color space that models human perception, and has significantly more accurate to perception lightness values, so this is an excellent color space for color operations that wish to preserve color brightness properly.  Traditionally, values are L [0,100], a,b [-200,+200] but here we normalize them all to the 0-1 range. If you hate it, let me know why!
---
# [Color]({{site.url}}/Pages/Reference/Color.html).LAB

<div class='signature' markdown='1'>
```csharp
static Color LAB(float l, float a, float b, float opacity)
```
Creates a gamma space RGB color from a CIE-L*ab color
space. CIE-L*ab is a color space that models human perception,
and has significantly more accurate to perception lightness
values, so this is an excellent color space for color operations
that wish to preserve color brightness properly.

Traditionally, values are L [0,100], a,b [-200,+200] but here we
normalize them all to the 0-1 range. If you hate it, let me know
why!
</div>

|  |  |
|--|--|
|float l|Lightness of the color! Range is 0-1.|
|float a|'a' is from red to green. Range is 0-1.|
|float b|'b' is from blue to yellow. Range is 0-1.|
|float opacity|The opacity copied into the final color!|
|RETURNS: [Color]({{site.url}}/Pages/Reference/Color.html)|A gamma space RGBA color constructed from the LAB values.|

<div class='signature' markdown='1'>
```csharp
static Color LAB(Vec3 lab, float opacity)
```
Creates a gamma space RGB color from a CIE-L*ab color
space. CIE-L*ab is a color space that models human perception,
and has significantly more accurate to perception lightness
values, so this is an excellent color space for color operations
that wish to preserve color brightness properly.

Traditionally, values are L [0,100], a,b [-200,+200] but here we
normalize them all to the 0-1 range. If you hate it, let me know
why!
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) lab|For convenience, XYZ is LAB.             Lightness of the color! Range is 0-1.             'a' is from red to green. Range is 0-1.             'b' is from blue to yellow. Range is 0-1.|
|float opacity|The opacity copied into the final color!|
|RETURNS: [Color]({{site.url}}/Pages/Reference/Color.html)|A gamma space RGBA color constructed from the LAB values.|





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

