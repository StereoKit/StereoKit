---
layout: default
title: Text.MakeStyle
description: Create a text style for use with other text functions! A text style is a font plus size/color/material parameters, and are used to keep text looking more consistent through the application by encouraging devs to re-use styles throughout the project.  This overload will create a unique Material for this style based on Default.ShaderFont.
---
# [Text]({{site.url}}/Pages/Reference/Text.html).MakeStyle

<div class='signature' markdown='1'>
```csharp
static TextStyle MakeStyle(Font font, float characterHeightMeters, Color colorGamma)
```
Create a text style for use with other text functions! A
text style is a font plus size/color/material parameters, and are
used to keep text looking more consistent through the application
by encouraging devs to re-use styles throughout the project.

This overload will create a unique Material for this style based
on Default.ShaderFont.
</div>

|  |  |
|--|--|
|[Font]({{site.url}}/Pages/Reference/Font.html) font|Font asset you want attached to this style.|
|float characterHeightMeters|Height of a text glyph in             meters. StereoKit currently bases this on the letter 'T'.|
|[Color]({{site.url}}/Pages/Reference/Color.html) colorGamma|The gamma space color of the text             style. This will be embedded in the vertex color of the text             mesh.|
|RETURNS: [TextStyle]({{site.url}}/Pages/Reference/TextStyle.html)|A text style id for use with text rendering functions.|

<div class='signature' markdown='1'>
```csharp
static TextStyle MakeStyle(Font font, float characterHeightMeters, Shader shader, Color colorGamma)
```
Create a text style for use with other text functions! A
text style is a font plus size/color/material parameters, and are
used to keep text looking more consistent through the application
by encouraging devs to re-use styles throughout the project.

This overload will create a unique Material for this style based
on the provided Shader.
</div>

|  |  |
|--|--|
|[Font]({{site.url}}/Pages/Reference/Font.html) font|Font asset you want attached to this style.|
|float characterHeightMeters|Height of a text glyph in             meters. StereoKit currently bases this on the letter 'T'.|
|[Shader]({{site.url}}/Pages/Reference/Shader.html) shader|This style will create and use a unique             Material based on the Shader that you provide here.|
|[Color]({{site.url}}/Pages/Reference/Color.html) colorGamma|The gamma space color of the text             style. This will be embedded in the vertex color of the text             mesh.|
|RETURNS: [TextStyle]({{site.url}}/Pages/Reference/TextStyle.html)|A text style id for use with text rendering functions.|

<div class='signature' markdown='1'>
```csharp
static TextStyle MakeStyle(Font font, float characterHeightMeters, Material material, Color colorGamma)
```
Create a text style for use with other text functions! A
text style is a font plus size/color/material parameters, and are
used to keep text looking more consistent through the application
by encouraging devs to re-use styles throughout the project.

This overload allows you to set the specific Material that is
used. This can be helpful if you're keeping styles similar enough
to re-use the material and save on draw calls. If you don't know
what that means, then prefer using the overload that takes a
Shader, or takes neither a Shader nor a Material!
</div>

|  |  |
|--|--|
|[Font]({{site.url}}/Pages/Reference/Font.html) font|Font asset you want attached to this style.|
|float characterHeightMeters|Height of a text glyph in             meters. StereoKit currently bases this on the letter 'T'.|
|[Material]({{site.url}}/Pages/Reference/Material.html) material|Which material should be used to render             the text with? Note that this does NOT duplicate the material, so             some parameters of this Material instance will get overwritten,              like the texture used for the glyph atlas. You should either use             a new Material, or a Material that was already used with this             same font.|
|[Color]({{site.url}}/Pages/Reference/Color.html) colorGamma|The gamma space color of the text             style. This will be embedded in the vertex color of the text             mesh.|
|RETURNS: [TextStyle]({{site.url}}/Pages/Reference/TextStyle.html)|A text style id for use with text rendering functions.|





## Examples

### Drawing text with and without a TextStyle
![Basic text]({{site.url}}/img/screenshots/BasicText.jpg)
We can use a TextStyle object to control how text gets displayed!
```csharp
TextStyle style;
```
In initialization, we can create the style from a font, a size,
and a base color. Overloads for MakeStyle can allow you to
override the default font shader, or provide a specific Material.
```csharp
style = Text.MakeStyle(
	Font.FromFile("C:/Windows/Fonts/times.ttf") ?? Default.Font, 
	2 * U.cm,
	Color.HSV(0.55f, 0.62f, 0.93f));
```
Then it's pretty trivial to just draw some text on the screen! Just call
Text.Add on update. If you don't have a TextStyle available, calling it
without one will just fall back on the default style.
```csharp
// Text with an explicit text style
Text.Add(
	"Here's\nSome\nMulti-line\nText!!", 
	Matrix.TR(new Vec3(0.1f, 0, 0), Quat.LookDir(0, 0, 1)),
	style);
// Text using the default text style
Text.Add(
	"Here's\nSome\nMulti-line\nText!!", 
	Matrix.TR(new Vec3(-0.1f, 0, 0), Quat.LookDir(0, 0, 1)));
```

