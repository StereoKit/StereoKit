---
layout: default
title: UI.Toggle
description: A toggleable button! A button will expand to fit the text provided to it, vertically and horizontally. Text is re-used as the id. Will return true any time the toggle value changes!
---
# [UI]({{site.url}}/Pages/Reference/UI.html).Toggle

<div class='signature' markdown='1'>
static bool Toggle(string text, Boolean& value)
</div>

|  |  |
|--|--|
|string text|Text to display on the button, should be per-             window unique as it will be used as the element id.|
|Boolean& value|The current state of the toggle button! True              means it's toggled on, and false means it's toggled off.|
|RETURNS: bool|Will return true any time the toggle value changes!|

A toggleable button! A button will expand to fit the
text provided to it, vertically and horizontally. Text is re-used
as the id. Will return true any time the toggle value changes!



