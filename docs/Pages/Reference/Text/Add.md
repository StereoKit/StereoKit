---
layout: default
title: Text.Add
description: Renders text at the given location! Must be called every frame you want this text to be visible.
---
# [Text]({{site.url}}/Pages/Reference/Text.html).Add

<div class='signature' markdown='1'>
static void Add(string text, [Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform, [TextStyle]({{site.url}}/Pages/Reference/TextStyle.html) style, TextAlign position, TextAlign align, float offX, float offY, float offZ)
</div>

|  |  |
|--|--|
|string text|What text should be drawn?|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform|A Matrix representing the transform of the text mesh! Try Matrix.TRS.|
|[TextStyle]({{site.url}}/Pages/Reference/TextStyle.html) style|Style information for rendering, see Text.MakeStyle or the TextStyle object.|
|TextAlign position|How should the text's bounding rectangle be positioned relative to the transform?|
|TextAlign align|How should the text be aligned within the text's bounding rectangle?|
|float offX|An additional offset on the X axis.|
|float offY|An additional offset on the Y axis.|
|float offZ|An additional offset on the Z axis.|

Renders text at the given location! Must be called every frame you want this text to be visible.
<div class='signature' markdown='1'>
static void Add(string text, [Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform, TextAlign position, TextAlign align, float offX, float offY, float offZ)
</div>

|  |  |
|--|--|
|string text|What text should be drawn?|
|[Matrix]({{site.url}}/Pages/Reference/Matrix.html) transform|A Matrix representing the transform of the text mesh! Try Matrix.TRS.|
|TextAlign position|How should the text's bounding rectangle be positioned relative to the transform?|
|TextAlign align|How should the text be aligned within the text's bounding rectangle?|
|float offX|An additional offset on the X axis.|
|float offY|An additional offset on the Y axis.|
|float offZ|An additional offset on the Z axis.|

Renders text at the given location! Must be called every frame you want this text to be visible.



