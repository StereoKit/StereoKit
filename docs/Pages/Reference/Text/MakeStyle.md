---
layout: default
title: Text.MakeStyle
description: Create a text style for use with other text functions! A text style is a font plus size/color/material parameters, and are used to keep text looking more consistent through the application by encouraging devs to re-use styles throughout the project.
---
# [Text]({{site.url}}/Pages/Reference/Text.html).MakeStyle

<div class='signature' markdown='1'>
static [TextStyle]({{site.url}}/Pages/Reference/TextStyle.html) MakeStyle([Font]({{site.url}}/Pages/Reference/Font.html) font, float characterHeight, [Material]({{site.url}}/Pages/Reference/Material.html) material, [Color32]({{site.url}}/Pages/Reference/Color32.html) color)
</div>

|  |  |
|--|--|
|[Font]({{site.url}}/Pages/Reference/Font.html) font|Font asset you want attached to this style.|
|float characterHeight|Height of a text glpyh in meters. TODO: find what text glyph specifically!|
|[Material]({{site.url}}/Pages/Reference/Material.html) material|Which material should be used to render the text with?|
|[Color32]({{site.url}}/Pages/Reference/Color32.html) color|The color of the text style. This will be embedded in the vertex color of the text mesh.|
|RETURNS: [TextStyle]({{site.url}}/Pages/Reference/TextStyle.html)|A text style id for use with text rendering functions.|

Create a text style for use with other text functions! A text style
is a font plus size/color/material parameters, and are used to keep text looking
more consistent through the application by encouraging devs to re-use styles
throughout the project.




## Examples

### Drawing text with and without a TextStyle
![Basic text]({{site.url}}/img/screenshots/BasicText.jpg)
We can use a TextStyle object to control how text gets displayed!
```csharp
TextStyle style;
```
In initialization, we can create the style from a font, a size, a
material, and a base color.
```csharp
style = Text.MakeStyle(
    Font.FromFile("C:/Windows/Fonts/Gabriola.ttf"), 
    2 * Units.cm2m,
    Material.Copy(DefaultIds.materialFont), 
    Color.HSV(0.05f, 0.7f, 0.8f));
```
Then it's pretty trivial to just draw some text on the screen! Just call
Text.Add on update. If you don't have a TextStyle available, calling it
without one will just fall back on the default style.
```csharp
// Text with an explicit text style
Text.Add(
    "Here's\nSome\nMulti-line\nText!!", 
    Matrix.TRS(new Vec3(0.1f, 0, 0), Quat.LookDir(0, 0, 1)),
    style);
// Text using the default text style
Text.Add(
    "Here's\nSome\nMulti-line\nText!!", 
    Matrix.TRS(new Vec3(-0.1f, 0, 0), Quat.LookDir(0, 0, 1)));
```

