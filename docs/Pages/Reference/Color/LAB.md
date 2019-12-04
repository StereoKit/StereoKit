---
layout: default
title: Color.LAB
description: Creates an RGB color from a CIE-L*ab color space. CIE-L*ab is a color space that models human perception, and has significantly more accurate to perception lightness values, so this is an excellent color space for color operations that wish to preserve color brightness properly. Traditionally, values are L [0,100], a,b [-200,+200] but here we normalize them all to the 0-1 range. If you hate it, let me know why!
---
# [Color]({{site.url}}/Pages/Reference/Color.html).LAB

<div class='signature' markdown='1'>
static [Color]({{site.url}}/Pages/Reference/Color.html) LAB(float l, float a, float b, float opacity)
</div>

|  |  |
|--|--|
|float l|Lightness of the color! Range is 0-1.|
|float a|'a' is from red to green. Range is 0-1.|
|float b|'b' is from blue to yellow. Range is 0-1.|
|float opacity|The opacity copied into the final color!|
|RETURNS: [Color]({{site.url}}/Pages/Reference/Color.html)|An RGBA color constructed from the LAB values.|

Creates an RGB color from a CIE-L*ab color space. CIE-L*ab is a color space that models
human perception, and has significantly more accurate to perception lightness values, so this is
an excellent color space for color operations that wish to preserve color brightness properly.
Traditionally, values are L [0,100], a,b [-200,+200] but here we normalize them all to the 0-1
range. If you hate it, let me know why!



