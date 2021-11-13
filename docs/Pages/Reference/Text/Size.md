---
layout: default
title: Text.Size
description: Sometimes you just need to know how much room some text takes up! This finds the size of the text in meters, when using the indicated style!
---
# [Text]({{site.url}}/Pages/Reference/Text.html).Size

<div class='signature' markdown='1'>
```csharp
static Vec2 Size(string text, TextStyle style)
```
Sometimes you just need to know how much room some text takes up! This finds
the size of the text in meters, when using the indicated style!
</div>

|  |  |
|--|--|
|string text|Text you want to find the size of.|
|[TextStyle]({{site.url}}/Pages/Reference/TextStyle.html) style|The visual style of the text, see Text.MakeStyle or the TextStyle object for more details.|
|RETURNS: [Vec2]({{site.url}}/Pages/Reference/Vec2.html)|The width and height of the text in meters.|

<div class='signature' markdown='1'>
```csharp
static Vec2 Size(string text)
```
Sometimes you just need to know how much room some text takes up! This finds
the size of the text in meters, when using the indicated style!
</div>

|  |  |
|--|--|
|string text|Text you want to find the size of.|
|RETURNS: [Vec2]({{site.url}}/Pages/Reference/Vec2.html)|The width and height of the text in meters.|




