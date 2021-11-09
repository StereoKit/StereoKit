---
layout: default
title: Input.TextReset
description: Resets the Input.TextConsume read list back to the start. For example, UI.Input will _not_ call TextReset, so it effectively will consume those characters, hiding them from any TextConsume calls following it. If you wanted to check the current frame's text, but still allow UI.Input to work later on in the frame, you would read everything with TextConsume, and then TextReset afterwards to reset the read list for the following UI.Input.
---
# [Input]({{site.url}}/Pages/Reference/Input.html).TextReset

<div class='signature' markdown='1'>
```csharp
static void TextReset()
```
Resets the `Input.TextConsume` read list back to the
start.
For example, `UI.Input` will _not_ call `TextReset`, so it
effectively will consume those characters, hiding them from
any `TextConsume` calls following it. If you wanted to check the
current frame's text, but still allow `UI.Input` to work later on
in the frame, you would read everything with `TextConsume`, and
then `TextReset` afterwards to reset the read list for the
following `UI.Input`.
</div>




