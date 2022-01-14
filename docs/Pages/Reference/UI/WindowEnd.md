---
layout: default
title: UI.WindowEnd
description: Finishes a window! Must be called after UI.WindowBegin() and all elements have been drawn.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).WindowEnd

<div class='signature' markdown='1'>
```csharp
static void WindowEnd()
```
Finishes a window! Must be called after UI.WindowBegin()
and all elements have been drawn.
</div>





## Examples

### A simple button

![A window with a button]({{site.screen_url}}/UI/ButtonWindow.jpg)

This is a complete window with a simple button on it! `UI.Button`
returns true only for the very first frame the button is pressed, so
using the `if(UI.Button())` pattern works very well for executing
code on button press!

```csharp
Pose windowPoseButton = new Pose(0, 0, 0, Quat.Identity);
void ShowWindowButton()
{
	UI.WindowBegin("Window Button", ref windowPoseButton);

	if (UI.Button("Press me!"))
		Log.Info("Button was pressed.");

	UI.WindowEnd();
}
```

