---
layout: default
title: Text.Add
description: Renders text at the given location! Must be called every frame you want this text to be visible.
---
# [Text]({{site.url}}/Pages/Reference/Text.html).Add

<div class='signature' markdown='1'>
```csharp
static void Add(string text, Matrix transform, TextStyle style, TextAlign position, TextAlign align, float offX, float offY, float offZ)
```
Renders text at the given location! Must be called every
frame you want this text to be visible.
</div>

|  |  |
|--|--|
|string text|What text should be drawn?|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform|A Matrix representing the transform of the             text mesh! Try Matrix.TRS.|
|[TextStyle]({{site.url}}/Pages/Reference/TextStyle.html) style|Style information for rendering, see             Text.MakeStyle or the TextStyle object.|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) position|How should the text's bounding rectangle be             positioned relative to the transform?|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) align|How should the text be aligned within the             text's bounding rectangle?|
|float offX|An additional offset on the X axis.|
|float offY|An additional offset on the Y axis.|
|float offZ|An additional offset on the Z axis.|

<div class='signature' markdown='1'>
```csharp
static void Add(string text, Matrix transform, TextAlign position, TextAlign align, float offX, float offY, float offZ)
```
Renders text at the given location! Must be called every
frame you want this text to be visible.
</div>

|  |  |
|--|--|
|string text|What text should be drawn?|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform|A Matrix representing the transform of the             text mesh! Try Matrix.TRS.|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) position|How should the text's bounding rectangle be             positioned relative to the transform?|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) align|How should the text be aligned within the             text's bounding rectangle?|
|float offX|An additional offset on the X axis.|
|float offY|An additional offset on the Y axis.|
|float offZ|An additional offset on the Z axis.|

<div class='signature' markdown='1'>
```csharp
static void Add(string text, Matrix transform, TextStyle style, Color vertexTintLinear, TextAlign position, TextAlign align, float offX, float offY, float offZ)
```
Renders text at the given location! Must be called every
frame you want this text to be visible.
</div>

|  |  |
|--|--|
|string text|What text should be drawn?|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform|A Matrix representing the transform of the             text mesh! Try Matrix.TRS.|
|[TextStyle]({{site.url}}/Pages/Reference/TextStyle.html) style|Style information for rendering, see             Text.MakeStyle or the TextStyle object.|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) position|How should the text's bounding rectangle be             positioned relative to the transform?|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) align|How should the text be aligned within the             text's bounding rectangle?|
|float offX|An additional offset on the X axis.|
|float offY|An additional offset on the Y axis.|
|float offZ|An additional offset on the Z axis.|
|[Color]({{site.url}}/Pages/Reference/Color.html) vertexTintLinear|The vertex color of the text gets             multiplied by this color. This is a linear color value, not a gamma             corrected color value.|

<div class='signature' markdown='1'>
```csharp
static void Add(string text, Matrix transform, Color vertexTintLinear, TextAlign position, TextAlign align, float offX, float offY, float offZ)
```
Renders text at the given location! Must be called every
frame you want this text to be visible.
</div>

|  |  |
|--|--|
|string text|What text should be drawn?|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform|A Matrix representing the transform of the             text mesh! Try Matrix.TRS.|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) position|How should the text's bounding rectangle be             positioned relative to the transform?|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) align|How should the text be aligned within the             text's bounding rectangle?|
|float offX|An additional offset on the X axis.|
|float offY|An additional offset on the Y axis.|
|float offZ|An additional offset on the Z axis.|
|[Color]({{site.url}}/Pages/Reference/Color.html) vertexTintLinear|The vertex color of the text gets             multiplied by this color. This is a linear color value, not a gamma             corrected color value.|

<div class='signature' markdown='1'>
```csharp
static float Add(string text, Matrix transform, Vec2 size, TextFit fit, TextStyle style, TextAlign position, TextAlign align, float offX, float offY, float offZ)
```
Renders text at the given location! Must be called every
frame you want this text to be visible.
</div>

|  |  |
|--|--|
|string text|What text should be drawn?|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform|A Matrix representing the transform of the             text mesh! Try Matrix.TRS.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) size|This is the Hierarchy space rectangle that the             text should try to fit inside of. This allows for text wrapping or             scaling based on the value provided to the 'fit' parameter.|
|[TextFit]({{site.url}}/Pages/Reference/TextFit.html) fit|Describe how the text should behave when one of             its size dimensions conflicts with the provided 'size' parameter.|
|[TextStyle]({{site.url}}/Pages/Reference/TextStyle.html) style|Style information for rendering, see             Text.MakeStyle or the TextStyle object.|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) position|How should the text's bounding rectangle be             positioned relative to the transform?|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) align|How should the text be aligned within the             text's bounding rectangle?|
|float offX|An additional offset on the X axis.|
|float offY|An additional offset on the Y axis.|
|float offZ|An additional offset on the Z axis.|
|RETURNS: float|Returns the vertical space used by this text.|

<div class='signature' markdown='1'>
```csharp
static float Add(string text, Matrix transform, Vec2 size, TextFit fit, TextAlign position, TextAlign align, float offX, float offY, float offZ)
```
Renders text at the given location! Must be called every
frame you want this text to be visible.
</div>

|  |  |
|--|--|
|string text|What text should be drawn?|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform|A Matrix representing the transform of the             text mesh! Try Matrix.TRS.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) size|This is the Hierarchy space rectangle that the             text should try to fit inside of. This allows for text wrapping or             scaling based on the value provided to the 'fit' parameter.|
|[TextFit]({{site.url}}/Pages/Reference/TextFit.html) fit|Describe how the text should behave when one of             its size dimensions conflicts with the provided 'size' parameter.|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) position|How should the text's bounding rectangle be             positioned relative to the transform?|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) align|How should the text be aligned within the             text's bounding rectangle?|
|float offX|An additional offset on the X axis.|
|float offY|An additional offset on the Y axis.|
|float offZ|An additional offset on the Z axis.|
|RETURNS: float|Returns the vertical space used by this text.|

<div class='signature' markdown='1'>
```csharp
static float Add(string text, Matrix transform, Vec2 size, TextFit fit, Color vertexTintLinear, TextAlign position, TextAlign align, float offX, float offY, float offZ)
```
Renders text at the given location! Must be called every
frame you want this text to be visible.
</div>

|  |  |
|--|--|
|string text|What text should be drawn?|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform|A Matrix representing the transform of the             text mesh! Try Matrix.TRS.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) size|This is the Hierarchy space rectangle that the             text should try to fit inside of. This allows for text wrapping or             scaling based on the value provided to the 'fit' parameter.|
|[TextFit]({{site.url}}/Pages/Reference/TextFit.html) fit|Describe how the text should behave when one of             its size dimensions conflicts with the provided 'size' parameter.|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) position|How should the text's bounding rectangle be             positioned relative to the transform?|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) align|How should the text be aligned within the             text's bounding rectangle?|
|float offX|An additional offset on the X axis.|
|float offY|An additional offset on the Y axis.|
|float offZ|An additional offset on the Z axis.|
|[Color]({{site.url}}/Pages/Reference/Color.html) vertexTintLinear|The vertex color of the text gets             multiplied by this color. This is a linear color value, not a gamma             corrected color value.|
|RETURNS: float|Returns the vertical space used by this text.|

<div class='signature' markdown='1'>
```csharp
static float Add(string text, Matrix transform, Vec2 size, TextFit fit, TextStyle style, Color vertexTintLinear, TextAlign position, TextAlign align, float offX, float offY, float offZ)
```
Renders text at the given location! Must be called every
frame you want this text to be visible.
</div>

|  |  |
|--|--|
|string text|What text should be drawn?|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform|A Matrix representing the transform of the             text mesh! Try Matrix.TRS.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) size|This is the Hierarchy space rectangle that the             text should try to fit inside of. This allows for text wrapping or             scaling based on the value provided to the 'fit' parameter.|
|[TextFit]({{site.url}}/Pages/Reference/TextFit.html) fit|Describe how the text should behave when one of             its size dimensions conflicts with the provided 'size' parameter.|
|[TextStyle]({{site.url}}/Pages/Reference/TextStyle.html) style|Style information for rendering, see             Text.MakeStyle or the TextStyle object.|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) position|How should the text's bounding rectangle be             positioned relative to the transform?|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) align|How should the text be aligned within the             text's bounding rectangle?|
|float offX|An additional offset on the X axis.|
|float offY|An additional offset on the Y axis.|
|float offZ|An additional offset on the Z axis.|
|[Color]({{site.url}}/Pages/Reference/Color.html) vertexTintLinear|The vertex color of the text gets             multiplied by this color. This is a linear color value, not a gamma             corrected color value.|
|RETURNS: float|Returns the vertical space used by this text.|





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

