---
layout: default
title: UI.Label
description: Adds some text to the layout! Text uses the UI's current font settings (which are currently not exposed). Can contain newlines! May have trouble with non-latin characters. Will advance layout to next line.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).Label

<div class='signature' markdown='1'>
```csharp
static void Label(string text, bool usePadding)
```
Adds some text to the layout! Text uses the UI's current
font settings (which are currently not exposed). Can contain
newlines! May have trouble with non-latin characters. Will
advance layout to next line.
</div>

|  |  |
|--|--|
|string text|Label text to display. Can contain newlines!             May have trouble with non-latin characters. Doesn't use text as             id, so it can be non-unique.|
|bool usePadding|Should padding be included for             positioning this text? Sometimes you just want un-padded text!|





## Examples

### Separating UI Visually

![A window with text and a separator]({{site.screen_url}}/UI/SeparatorWindow.jpg)

Toggle buttons swap between true and false when you press them! The
function requires a reference to a bool variable where the toggle's
state is stored. This allows you to manage the state yourself, and
it's completely valid for you to change the toggle state separately,
the UI element will update to match.

Note that `UI.Toggle` returns true _only_ when the toggle state has
changed, and does _not_ return the current state.

```csharp
Pose windowPoseSeparator = new Pose(.6f, 0, 0, Quat.Identity);
void ShowWindowSeparator()
{
	UI.WindowBegin("Window Separator", ref windowPoseSeparator, UIWin.Body);

	UI.Label("Content Header");
	UI.HSeparator();
	UI.Text("A separator can go a long way towards making your content "
	      + "easier to look at!", TextAlign.TopCenter);

	UI.WindowEnd();
}
```

