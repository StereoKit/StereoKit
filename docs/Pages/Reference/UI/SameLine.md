---
layout: default
title: UI.SameLine
description: Moves the current layout position back to the end of the line that just finished, so it can continue on the same line as the last element!
---
# [UI]({{site.url}}/Pages/Reference/UI.html).SameLine

<div class='signature' markdown='1'>
```csharp
static void SameLine()
```
Moves the current layout position back to the end of the
line that just finished, so it can continue on the same line as the
last element!
</div>





## Examples

### Text Input

![A window with a text input]({{site.screen_url}}/UI/InputWindow.jpg)

A slider will slide between two values at increments. The function
requires a reference to a float variable where the slider's state is
stored. This allows you to manage the state yourself, and it's
completely valid for you to change the slider state separately, the
UI element will update to match.

Note that `UI.HSlider` returns true _only_ when the slider state has
changed, and does _not_ return the current state.

```csharp
Pose   windowPoseInput = new Pose(1.2f, 0, 0, Quat.Identity);
string inputState = "Initial text";
void ShowWindowInput()
{
	UI.WindowBegin("Window Input", ref windowPoseInput);

	// Add a small label in front of it on the same line
	UI.Label("Text:");
	UI.SameLine();
	if (UI.Input("Text", ref inputState))
		Log.Info($"Input text just changed");

	UI.WindowEnd();
}
```

