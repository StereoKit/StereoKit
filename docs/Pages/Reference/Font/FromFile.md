---
layout: default
title: Font.FromFile
description: Loads a font and creates a font asset from it.
---
# [Font]({{site.url}}/Pages/Reference/Font.html).FromFile

<div class='signature' markdown='1'>
static [Font]({{site.url}}/Pages/Reference/Font.html) FromFile(string fontFile)
</div>

|  |  |
|--|--|
|string fontFile|A file address for the font! For example: 'C:/Windows/Fonts/segoeui.ttf'|
|RETURNS: [Font]({{site.url}}/Pages/Reference/Font.html)|A font from the given file, or null if the file failed to load properly!|

Loads a font and creates a font asset from it.




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
	Font.FromFile("C:/Windows/Fonts/Arial.ttf") ?? Default.Font, 
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

