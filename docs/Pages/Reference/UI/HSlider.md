---
layout: default
title: UI.HSlider
description: A horizontal slider element! You can stick your finger in it, and slide the value up and down.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).HSlider

<div class='signature' markdown='1'>
static bool HSlider(string id, Single& value, float min, float max, float step, float width, [UIConfirm]({{site.url}}/Pages/Reference/UIConfirm.html) confirmMethod)
</div>

|  |  |
|--|--|
|string id|A per-window unique id for tracking element              state.|
|Single& value|The value that the slider will store slider              state in.|
|float min|The minimum value the slider can set, left side              of the slider.|
|float max|The maximum value the slider can set, right              side of the slider.|
|float step|Locks the value to intervals of step. Starts              at min, and increments by step.|
|float width|Physical width of the slider on the window.|
|[UIConfirm]({{site.url}}/Pages/Reference/UIConfirm.html) confirmMethod|How should the slider be activated?             Push will be a push-button the user must press first, and pinch             will be a tab that the user must pinch and drag around.|
|RETURNS: bool|Returns true any time the value changes.|

A horizontal slider element! You can stick your finger
in it, and slide the value up and down.



