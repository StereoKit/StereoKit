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
|Empty|No body, no head. Not really a flag, just set to this value. The Window will still be grab/movable. To prevent it from being grabbable, combine with the UIMove.None option, or switch to UI.Push/PopSurface.|
|Head|Flag to include a head on the window.|
|Normal|A normal window has a head and a body to it. Both can be grabbed.|

## Examples

### Separating UI Visually

![A window with text and a separator]({{site.screen_url}}/UI/SeparatorWindow.jpg)

A separator is a simple visual element that fills the window
horizontally. It's nothing complicated, but can help create visual
association between groups of UI elements.

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

