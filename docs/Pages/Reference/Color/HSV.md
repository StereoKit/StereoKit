---
layout: default
title: Color.HSV
description: Creates a Red/Green/Blue color from Hue/Saturation/Value information.
---
# [Color]({{site.url}}/Pages/Reference/Color.html).HSV

<div class='signature' markdown='1'>
static [Color]({{site.url}}/Pages/Reference/Color.html) HSV(float hue, float saturation, float value, float opacity)
</div>

|  |  |
|--|--|
|float hue|Hue most directly relates to the color as we think of it! 0 is red,             0.1667 is yellow, 0.3333 is green, 0.5 is cyan, 0.6667 is blue, 0.8333 is magenta, and 1             is red again!|
|float saturation|The vibrancy of the color, where 0 is straight up a shade of gray,             and 1 is 'poke you in the eye colorful'.|
|float value|The brightness of the color! 0 is always black.|
|float opacity|Also known as alpha! This is does not affect the rgb components of the             resulting color, it'll just get slotted into the colors opacity value.|
|RETURNS: [Color]({{site.url}}/Pages/Reference/Color.html)||

Creates a Red/Green/Blue color from Hue/Saturation/Value information.



