---
layout: default
title: UIWin
description: A description of what type of window to draw! This is a bit flag, so it can contain multiple elements.
---
# enum UIWin

A description of what type of window to draw! This is a bit
flag, so it can contain multiple elements.




## Enum Values

|  |  |
|--|--|
|Body|Flag to include a body on the window.|
|Empty|No body, no head, none of the window will be grabbable. Not really a flag, just set to this value.|
|Head|Flag to include a head on the window.|
|Normal|A normal window has a head and a body to it. Both can be grabbed.|



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

