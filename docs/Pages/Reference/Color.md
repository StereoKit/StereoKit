---
layout: default
title: Color
description: A color value stored as 4 floats with values that are generally between 0 and 1! Note that there's also a Color32 structure, and that 4 floats is generally a lot more than you need. So, use this for calculating individual colors at quality, but maybe store them en-masse with Color32!  Also note that RGB is often a terrible color format for picking colors, but it's how our displays work and we're stuck with it. If you want to create a color via code, try out the static Color.HSV method instead!
---
# Color

A color value stored as 4 floats with values that are generally between
0 and 1! Note that there's also a Color32 structure, and that 4 floats is generally
a lot more than you need. So, use this for calculating individual colors at quality,
but maybe store them en-masse with Color32!

Also note that RGB is often a terrible color format for picking colors, but it's how
our displays work and we're stuck with it. If you want to create a color via code,
try out the static Color.HSV method instead!


## Fields and Properties

|  |  |
|--|--|
|float [a]({{site.url}}/Pages/Reference/Color/a.html)|Alpha, or opacity component, a value that is generally between 0-1, where 0 is completely transparent, and 1 is completely opaque.|
|float [b]({{site.url}}/Pages/Reference/Color/b.html)|Blue component, a value that is generally between 0-1|
|[Color]({{site.url}}/Pages/Reference/Color.html) [Black]({{site.url}}/Pages/Reference/Color/Black.html)|Pure opaque black! Same as (0,0,0,1).|
|[Color]({{site.url}}/Pages/Reference/Color.html) [BlackTransparent]({{site.url}}/Pages/Reference/Color/BlackTransparent.html)|Pure transparent black! Same as (0,0,0,0).|
|float [g]({{site.url}}/Pages/Reference/Color/g.html)|Green component, a value that is generally between 0-1|
|float [r]({{site.url}}/Pages/Reference/Color/r.html)|Red component, a value that is generally between 0-1|
|[Color]({{site.url}}/Pages/Reference/Color.html) [White]({{site.url}}/Pages/Reference/Color/White.html)|Pure opaque white! Same as (1,1,1,1).|



## Methods

|  |  |
|--|--|
|[Color]({{site.url}}/Pages/Reference/Color/Color.html)|Try Color.HSV instead! But if you really need to create a color from RGB values, I suppose you're in the right place. All parameter values are generally in the range of 0-1.|
|[HSV]({{site.url}}/Pages/Reference/Color/HSV.html)|Creates a Red/Green/Blue color from Hue/Saturation/Value information.|
|[LAB]({{site.url}}/Pages/Reference/Color/LAB.html)|Creates an RGB color from a CIE-L*ab color space. CIE-L*ab is a color space that models human perception, and has significantly more accurate to perception lightness values, so this is an excellent color space for color operations that wish to preserve color brightness properly. Traditionally, values are L [0,100], a,b [-200,+200] but here we normalize them all to the 0-1 range. If you hate it, let me know why!|
|[ToHSV]({{site.url}}/Pages/Reference/Color/ToHSV.html)|Converts the color to a Hue/Saturation/Value format! Does not consider transparency when calculating the result.|
|[ToLAB]({{site.url}}/Pages/Reference/Color/ToLAB.html)|Converts the RGB color to a CIE LAB color space value! Conversion back and forth from LAB space could be somewhat lossy.|


