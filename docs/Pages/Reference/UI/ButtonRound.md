---
layout: default
title: UI.ButtonRound
description: A pressable button! A button will expand to fit the text provided to it, vertically and horizontally. Text is re-used as the id. Will return true only on the first frame it is pressed!
---
# [UI]({{site.url}}/Pages/Reference/UI.html).ButtonRound

<div class='signature' markdown='1'>
```csharp
static bool ButtonRound(string id, Sprite image, float diameter)
```
A pressable button! A button will expand to fit the text
provided to it, vertically and horizontally. Text is re-used as the
id. Will return true only on the first frame it is pressed!
</div>

|  |  |
|--|--|
|string id|An id for tracking element state. MUST be unique             within current hierarchy.|
|[Sprite]({{site.url}}/Pages/Reference/Sprite.html) image|An image to display as the face of the button.|
|float diameter|The diameter of the button's visual.|
|RETURNS: bool|Will return true only on the first frame it is pressed!|




