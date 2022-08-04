---
layout: default
title: Font.FromFile
description: Loads a font and creates a font asset from it.
---
# [Font]({{site.url}}/Pages/Reference/Font.html).FromFile

<div class='signature' markdown='1'>
```csharp
static Font FromFile(String[] fontFiles)
```
Loads a font and creates a font asset from it.
</div>

|  |  |
|--|--|
|String[] fontFiles|A list of file addresses for the font! For             example: 'C:/Windows/Fonts/segoeui.ttf'. If a glyph is not found,             StereoKit will look in the next font file in the list.|
|RETURNS: [Font]({{site.url}}/Pages/Reference/Font.html)|A font from the given files, or null if all of the files failed to load properly! If any of the given files successfully loads, then this font will be a valid asset.|





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

