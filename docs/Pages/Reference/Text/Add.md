---
layout: default
title: Text.Add
description: Renders text at the given location! Must be called every frame you want this text to be visible.
---
# [Text]({{site.url}}/Pages/Reference/Text.html).Add

<div class='signature' markdown='1'>
static void Add(string text, [Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform, [TextStyle]({{site.url}}/Pages/Reference/TextStyle.html) style, [TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) position, [TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) align, float offX, float offY, float offZ)
</div>

|  |  |
|--|--|
|string text|What text should be drawn?|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform|A Matrix representing the transform of the text mesh! Try Matrix.TRS.|
|[TextStyle]({{site.url}}/Pages/Reference/TextStyle.html) style|Style information for rendering, see Text.MakeStyle or the TextStyle object.|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) position|How should the text's bounding rectangle be positioned relative to the transform?|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) align|How should the text be aligned within the text's bounding rectangle?|
|float offX|An additional offset on the X axis.|
|float offY|An additional offset on the Y axis.|
|float offZ|An additional offset on the Z axis.|

Renders text at the given location! Must be called every frame you want this text to be visible.
<div class='signature' markdown='1'>
static void Add(string text, [Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform, [TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) position, [TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) align, float offX, float offY, float offZ)
</div>

|  |  |
|--|--|
|string text|What text should be drawn?|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform|A Matrix representing the transform of the text mesh! Try Matrix.TRS.|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) position|How should the text's bounding rectangle be positioned relative to the transform?|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) align|How should the text be aligned within the text's bounding rectangle?|
|float offX|An additional offset on the X axis.|
|float offY|An additional offset on the Y axis.|
|float offZ|An additional offset on the Z axis.|

Renders text at the given location! Must be called every frame you want this text to be visible.




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

