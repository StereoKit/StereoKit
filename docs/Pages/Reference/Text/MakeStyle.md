---
layout: default
title: Text.MakeStyle
description: Create a text style for use with other text functions! A text style is a font plus size/color/material parameters, and are used to keep text looking more consistent through the application by encouraging devs to re-use styles throughout the project.
---
# [Text]({{site.url}}/Pages/Reference/Text.html).MakeStyle

<div class='signature' markdown='1'>
static TextStyle MakeStyle([Font]({{site.url}}/Pages/Reference/Font.html) font, float characterHeight, [Material]({{site.url}}/Pages/Reference/Material.html) material, [Color32]({{site.url}}/Pages/Reference/Color32.html) color)
</div>

|  |  |
|--|--|
|[Font]({{site.url}}/Pages/Reference/Font.html) font|Font asset you want attached to this style.|
|float characterHeight|Height of a text glpyh in meters. TODO: find what text glyph specifically!|
|[Material]({{site.url}}/Pages/Reference/Material.html) material|Which material should be used to render the text with?|
|[Color32]({{site.url}}/Pages/Reference/Color32.html) color|The color of the text style. This will be embedded in the vertex color of the text mesh.|

Create a text style for use with other text functions! A text style
is a font plus size/color/material parameters, and are used to keep text looking
more consistent through the application by encouraging devs to re-use styles
throughout the project.



