---
layout: default
title: Tex.GenColor
description: This generates a solid color texture of the given dimensions. Can be quite nice for creating placeholder textures! Make sure to match linear/gamma colors with the correct format.
---
# [Tex]({{site.url}}/Pages/Reference/Tex.html).GenColor

<div class='signature' markdown='1'>
```csharp
static Tex GenColor(Color color, int width, int height, TexType type, TexFormat format)
```
This generates a solid color texture of the given
dimensions. Can be quite nice for creating placeholder textures!
Make sure to match linear/gamma colors with the correct format.
</div>

|  |  |
|--|--|
|[Color]({{site.url}}/Pages/Reference/Color.html) color|The color to use for the texture. This is             interpreted slightly differently based on what TexFormat gets used.|
|int width|Width of the final texture, in pixels.|
|int height|Height of the final texture, in pixels.|
|[TexType]({{site.url}}/Pages/Reference/TexType.html) type|Not all types here are applicable, but             TexType.Image or TexType.ImageNomips are good options here.|
|[TexFormat]({{site.url}}/Pages/Reference/TexFormat.html) format|Not all formats are supported, but this does             support a decent range. The provided color is interpreted slightly             different depending on this format.|
|RETURNS: [Tex]({{site.url}}/Pages/Reference/Tex.html)|A solid color image of width x height pixels.|




