---
layout: default
title: UI.HSliderAt
description: A variant of UI.HSlider that doesn't use the layout system, and instead goes exactly where you put it.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).HSliderAt

<div class='signature' markdown='1'>
```csharp
static bool HSliderAt(string id, Single& value, float min, float max, float step, Vec3 topLeftCorner, Vec2 size, UIConfirm confirmMethod)
```
A variant of UI.HSlider that doesn't use the layout
system, and instead goes exactly where you put it.
</div>

|  |  |
|--|--|
|string id|An id for tracking element state. MUST be unique             within current hierarchy.|
|Single& value|The value that the slider will store slider              state in.|
|float min|The minimum value the slider can set, left side              of the slider.|
|float max|The maximum value the slider can set, right              side of the slider.|
|float step|Locks the value to intervals of step. Starts              at min, and increments by step.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) topLeftCorner|This is the top left corner of the UI             element relative to the current Hierarchy.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) size|The layout size for this element in Hierarchy             space. If an axis is left as zero, it will be auto-calculated. For             X this is the remaining width of the current layout, and for Y this             is UI.LineHeight.|
|[UIConfirm]({{site.url}}/Pages/Reference/UIConfirm.html) confirmMethod|How should the slider be activated?             Push will be a push-button the user must press first, and pinch             will be a tab that the user must pinch and drag around.|
|RETURNS: bool|Returns true any time the value changes.|

<div class='signature' markdown='1'>
```csharp
static bool HSliderAt(string id, Double& value, double min, double max, double step, Vec3 topLeftCorner, Vec2 size, UIConfirm confirmMethod)
```
A variant of UI.HSlider that doesn't use the layout
system, and instead goes exactly where you put it.
</div>

|  |  |
|--|--|
|string id|An id for tracking element state. MUST be unique             within current hierarchy.|
|Double& value|The value that the slider will store slider              state in.|
|double min|The minimum value the slider can set, left side              of the slider.|
|double max|The maximum value the slider can set, right              side of the slider.|
|double step|Locks the value to intervals of step. Starts              at min, and increments by step.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) topLeftCorner|This is the top left corner of the UI             element relative to the current Hierarchy.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) size|The layout size for this element in Hierarchy             space. If an axis is left as zero, it will be auto-calculated. For             X this is the remaining width of the current layout, and for Y this             is UI.LineHeight.|
|[UIConfirm]({{site.url}}/Pages/Reference/UIConfirm.html) confirmMethod|How should the slider be activated?             Push will be a push-button the user must press first, and pinch             will be a tab that the user must pinch and drag around.|
|RETURNS: bool|Returns true any time the value changes.|




