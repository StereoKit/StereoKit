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

