---
layout: default
title: UI.Toggle
description: A toggleable button! A button will expand to fit the text provided to it, vertically and horizontally. Text is re-used as the id. Will return true any time the toggle value changes!
---
# [UI]({{site.url}}/Pages/Reference/UI.html).Toggle

<div class='signature' markdown='1'>
```csharp
static bool Toggle(string text, Boolean& value)
```
A toggleable button! A button will expand to fit the
text provided to it, vertically and horizontally. Text is re-used
as the id. Will return true any time the toggle value changes!
</div>

|  |  |
|--|--|
|string text|Text to display on the Toggle and id for             tracking element state. MUST be unique within current hierarchy.|
|Boolean& value|The current state of the toggle button! True              means it's toggled on, and false means it's toggled off.|
|RETURNS: bool|Will return true any time the toggle value changes!|




