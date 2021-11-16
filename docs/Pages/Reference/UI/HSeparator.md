---
layout: default
title: UI.HSeparator
description: This draws a line horizontally across the current layout. Makes a good separator between sections of UI!
---
# [UI]({{site.url}}/Pages/Reference/UI.html).HSeparator

<div class='signature' markdown='1'>
```csharp
static void HSeparator()
```
This draws a line horizontally across the current
layout. Makes a good separator between sections of UI!
</div>





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

