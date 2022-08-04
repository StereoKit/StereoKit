---
layout: default
title: UI.ButtonImg
description: A pressable button accompanied by an image! The button will expand to fit the text provided to it, horizontally. Text is re-used as the id. Will return true only on the first frame it is pressed!
---
# [UI]({{site.url}}/Pages/Reference/UI.html).ButtonImg

<div class='signature' markdown='1'>
```csharp
static bool ButtonImg(string text, Sprite image, UIBtnLayout imageLayout)
```
A pressable button accompanied by an image! The button
will expand to fit the text provided to it, horizontally. Text is
re-used as the id. Will return true only on the first frame it is
pressed!
</div>

|  |  |
|--|--|
|string text|Text to display on the button and id for             tracking element state. MUST be unique within current hierarchy.|
|[Sprite]({{site.url}}/Pages/Reference/Sprite.html) image|This is the image that will be drawn along with             the text. See imageLayout for where the image gets drawn!|
|[UIBtnLayout]({{site.url}}/Pages/Reference/UIBtnLayout.html) imageLayout|This enum specifies how the text and             image should be laid out on the button. For example, `UIBtnLayout.Left`             will have the image on the left, and text on the right.|
|RETURNS: bool|Will return true only on the first frame it is pressed!|

<div class='signature' markdown='1'>
```csharp
static bool ButtonImg(string text, Sprite image, UIBtnLayout imageLayout, Vec2 size)
```
A pressable button accompanied by an image! The button
will expand to fit the text provided to it, horizontally. Text is
re-used as the id. Will return true only on the first frame it is
pressed!
</div>

|  |  |
|--|--|
|string text|Text to display on the button and id for             tracking element state. MUST be unique within current hierarchy.|
|[Sprite]({{site.url}}/Pages/Reference/Sprite.html) image|This is the image that will be drawn along with             the text. See imageLayout for where the image gets drawn!|
|[UIBtnLayout]({{site.url}}/Pages/Reference/UIBtnLayout.html) imageLayout|This enum specifies how the text and             image should be laid out on the button. For example, `UIBtnLayout.Left`             will have the image on the left, and text on the right.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) size|The layout size for this element in Hierarchy             space. If an axis is left as zero, it will be auto-calculated. For             X this is the remaining width of the current layout, and for Y this             is UI.LineHeight.|
|RETURNS: bool|Will return true only on the first frame it is pressed!|




