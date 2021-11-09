---
layout: default
title: UI.Radio
description: A Radio is similar to a button, except you can specify if it looks pressed or not regardless of interaction. This can be useful for radio-like behavior! Check an enum for a value, and use that as the 'active' state, Then switch to that enum value if Radio returns true.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).Radio

<div class='signature' markdown='1'>
```csharp
static bool Radio(string text, bool active)
```
A Radio is similar to a button, except you can specify if
it looks pressed or not regardless of interaction. This can be
useful for radio-like behavior! Check an enum for a value, and use
that as the 'active' state, Then switch to that enum value if Radio
returns true.
</div>

|  |  |
|--|--|
|string text|Text to display on the Radio and id for             tracking element state. MUST be unique within current hierarchy.|
|bool active|Does this button look like it's pressed?|
|RETURNS: bool|Will return true only on the first frame it is pressed!|




