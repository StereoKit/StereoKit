---
layout: default
title: Color.HSV
description: Creates a Red/Green/Blue gamma space color from Hue/Saturation/Value information.
---
# [Color]({{site.url}}/Pages/Reference/Color.html).HSV

<div class='signature' markdown='1'>
```csharp
static Color HSV(float hue, float saturation, float value, float opacity)
```
Creates a Red/Green/Blue gamma space color from
Hue/Saturation/Value information.
</div>

|  |  |
|--|--|
|float hue|Hue most directly relates to the color as we             think of it! 0 is red, 0.1667 is yellow, 0.3333 is green, 0.5 is             cyan, 0.6667 is blue, 0.8333 is magenta, and 1 is red again!|
|float saturation|The vibrancy of the color, where 0 is             straight up a shade of gray, and 1 is 'poke you in the eye             colorful'.|
|float value|The brightness of the color! 0 is always             black.|
|float opacity|Also known as alpha! This is does not             affect the rgb components of the resulting color, it'll just get             slotted into the colors opacity value.|
|RETURNS: [Color]({{site.url}}/Pages/Reference/Color.html)|A gamma space RGB color!|

<div class='signature' markdown='1'>
```csharp
static Color HSV(Vec3 hsvColor, float opacity)
```
Creates a Red/Green/Blue gamma space color from
Hue/Saturation/Value information.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) hsvColor|For convenience, XYZ is HSV.                          Hue most directly relates to the color as we think of it! 0 is             red, 0.1667 is yellow, 0.3333 is green, 0.5 is cyan, 0.6667 is             blue, 0.8333 is magenta, and 1 is red again!                          Saturation is the vibrancy of the color, where 0 is straight up a             shade of gray, and 1 is 'poke you in the eye colorful'.                          Value is the brightness of the color! 0 is always black.|
|float opacity|Also known as alpha! This is does not             affect the rgb components of the resulting color, it'll just get             slotted into the colors opacity value.|
|RETURNS: [Color]({{site.url}}/Pages/Reference/Color.html)|A gamma space RGB color!|





## Examples

```csharp
// You can create a color using Red, Green, Blue, Alpha values,
// but it's often a great recipe for making a bad color.
Color color = new Color(1,0,0,1); // Red

// Hue, Saturation, Value, Alpha is a more natural way of picking
// colors. The commentdocs have a list of important values for Hue,
// to make it a little easier to pick the hue you want.
color = Color.HSV(0, 1, 1, 1); // Red

// And there's a few static colors available if you need 'em.
color = Color.White;

// You can also implicitly convert Color to a Color32!
Color32 color32 = color;
```
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

