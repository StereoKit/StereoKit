---
layout: default
title: Color
description: A color value stored as 4 floats with values that are generally between 0 and 1! Note that there's also a Color32 structure, and that 4 floats is generally a lot more than you need. So, use this for calculating individual colors at quality, but maybe store them en-masse with Color32!  Also note that RGB is often a terrible color format for picking colors, but it's how our displays work and we're stuck with it. If you want to create a color via code, try out the static Color.HSV method instead!  A note on gamma space vs. linear space colors! Color is not inherently one or the other, but most color values we work with tend to be gamma space colors, so most functions in StereoKit are gamma space. There are occasional functions that will ask for linear space colors instead, primarily in performance critical places, or places where a color may not always be a color! However, performing math on gamma space colors is bad, and will result in incorrect colors. We do our best to indicate what color space a function uses, but it's not enforced through syntax!
---
# struct Color

A color value stored as 4 floats with values that are
generally between 0 and 1! Note that there's also a Color32
structure, and that 4 floats is generally a lot more than you need.
So, use this for calculating individual colors at quality, but maybe
store them en-masse with Color32!

Also note that RGB is often a terrible color format for picking
colors, but it's how our displays work and we're stuck with it. If
you want to create a color via code, try out the static Color.HSV
method instead!

A note on gamma space vs. linear space colors! `Color` is not
inherently one or the other, but most color values we work with tend
to be gamma space colors, so most functions in StereoKit are gamma
space. There are occasional functions that will ask for linear space
colors instead, primarily in performance critical places, or places
where a color may not always be a color! However, performing math on
gamma space colors is bad, and will result in incorrect colors. We do
our best to indicate what color space a function uses, but it's not
enforced through syntax!

## Instance Fields and Properties

|  |  |
|--|--|
|float [a]({{site.url}}/Pages/Reference/Color/a.html)|Alpha, or opacity component, a value that is generally between 0-1, where 0 is completely transparent, and 1 is completely opaque.|
|float [b]({{site.url}}/Pages/Reference/Color/b.html)|Blue component, a value that is generally between 0-1|
|float [g]({{site.url}}/Pages/Reference/Color/g.html)|Green component, a value that is generally between 0-1|
|float [r]({{site.url}}/Pages/Reference/Color/r.html)|Red component, a value that is generally between 0-1|

## Instance Methods

|  |  |
|--|--|
|[Color]({{site.url}}/Pages/Reference/Color/Color.html)|Try Color.HSV instead! But if you really need to create a color from RGB values, I suppose you're in the right place. All parameter values are generally in the range of 0-1.|
|[ToGamma]({{site.url}}/Pages/Reference/Color/ToGamma.html)|Converts this from a linear space color, into a gamma space color! If this is not a linear space color, this will just make your color wacky!|
|[ToHSV]({{site.url}}/Pages/Reference/Color/ToHSV.html)|Converts the gamma space color to a Hue/Saturation/Value format! Does not consider transparency when calculating the result.|
|[ToLAB]({{site.url}}/Pages/Reference/Color/ToLAB.html)|Converts the gamma space RGB color to a CIE LAB color space value! Conversion back and forth from LAB space could be somewhat lossy.|
|[ToLinear]({{site.url}}/Pages/Reference/Color/ToLinear.html)|Converts this from a gamma space color, into a linear space color! If this is not a gamma space color, this will just make your color wacky!|
|[ToString]({{site.url}}/Pages/Reference/Color/ToString.html)|Mostly for debug purposes, this is a decent way to log or inspect the color in debug mode. Looks like "[r, g, b, a]"|

## Static Fields and Properties

|  |  |
|--|--|
|[Color]({{site.url}}/Pages/Reference/Color.html) [Black]({{site.url}}/Pages/Reference/Color/Black.html)|Pure opaque black! Same as (0,0,0,1).|
|[Color]({{site.url}}/Pages/Reference/Color.html) [BlackTransparent]({{site.url}}/Pages/Reference/Color/BlackTransparent.html)|Pure transparent black! Same as (0,0,0,0).|
|[Color]({{site.url}}/Pages/Reference/Color.html) [White]({{site.url}}/Pages/Reference/Color/White.html)|Pure opaque white! Same as (1,1,1,1).|

## Static Methods

|  |  |
|--|--|
|[Hex]({{site.url}}/Pages/Reference/Color/Hex.html)|Create a color from an integer based hex value! This can make it easier to copy over colors from the web. This isn't a string function though, so you'll need to fill it out the whole way. Ex: `Color.Hex(0x0000FFFF)` would be RGBA(0,0,1,1).|
|[HSV]({{site.url}}/Pages/Reference/Color/HSV.html)|Creates a Red/Green/Blue gamma space color from Hue/Saturation/Value information.|
|[LAB]({{site.url}}/Pages/Reference/Color/LAB.html)|Creates a gamma space RGB color from a CIE-L*ab color space. CIE-L*ab is a color space that models human perception, and has significantly more accurate to perception lightness values, so this is an excellent color space for color operations that wish to preserve color brightness properly.  Traditionally, values are L [0,100], a,b [-200,+200] but here we normalize them all to the 0-1 range. If you hate it, let me know why!|

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

### Creating color from hex values
```csharp
Color   hex128 = Color  .Hex(0xFF0000FF); // Opaque Red
Color32 hex32  = Color32.Hex(0x00FF00FF); // Opaque Green
```

