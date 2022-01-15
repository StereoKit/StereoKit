---
layout: default
title: Platform.KeyboardShow
description: Request or hide a soft keyboard for the user to type on. StereoKit will surface OS provided soft keyboards where available, and use a fallback keyboard when not. On systems with physical keyboards, soft keyboards generally will not be shown if the user has interacted with their physical keyboard recently.
---
# [Platform]({{site.url}}/Pages/Reference/Platform.html).KeyboardShow

<div class='signature' markdown='1'>
```csharp
static void KeyboardShow(bool show, TextContext inputType)
```
Request or hide a soft keyboard for the user to type on.
StereoKit will surface OS provided soft keyboards where available,
and use a fallback keyboard when not. On systems with physical
keyboards, soft keyboards generally will not be shown if the user
has interacted with their physical keyboard recently.
</div>

|  |  |
|--|--|
|bool show|Tells whether or not to open or close the soft             keyboard.|
|[TextContext]({{site.url}}/Pages/Reference/TextContext.html) inputType|Soft keyboards can change layout to             optimize for the type of text that's required. StereoKit will             request the soft keyboard layout that most closely represents the             TextContext provided.|




