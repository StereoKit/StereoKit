---
layout: default
title: UI.PushPreserveKeyboard
description: When a soft keyboard is visible, interacting with UI elements will cause the keyboard to close. This function allows you to change this behavior for certain UI elements, allowing the user to interact and still preserve the keyboard's presence. Remember to Pop when you're finished!
---
# [UI]({{site.url}}/Pages/Reference/UI.html).PushPreserveKeyboard

<div class='signature' markdown='1'>
```csharp
static void PushPreserveKeyboard(bool preserveKeyboard)
```
When a soft keyboard is visible, interacting with UI
elements will cause the keyboard to close. This function allows you
to change this behavior for certain UI elements, allowing the user
to interact and still preserve the keyboard's presence. Remember
to Pop when you're finished!
</div>

|  |  |
|--|--|
|bool preserveKeyboard|If true, interacting with elements             will NOT hide the keyboard. If false, interaction will hide the             keyboard.|




