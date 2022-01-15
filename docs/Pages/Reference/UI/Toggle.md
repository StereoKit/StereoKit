---
layout: default
title: UI.Toggle
description: A toggleable button! A button will expand to fit the text provided to it, vertically and horizontally. Text is re-used as the id. Will return true any time the toggle value changes, NOT the toggle value itself!
---
# [UI]({{site.url}}/Pages/Reference/UI.html).Toggle

<div class='signature' markdown='1'>
```csharp
static bool Toggle(string text, Boolean& value)
```
A toggleable button! A button will expand to fit the
text provided to it, vertically and horizontally. Text is re-used
as the id. Will return true any time the toggle value changes, NOT
the toggle value itself!
</div>

|  |  |
|--|--|
|string text|Text to display on the Toggle and id for             tracking element state. MUST be unique within current hierarchy.|
|Boolean& value|The current state of the toggle button! True              means it's toggled on, and false means it's toggled off.|
|RETURNS: bool|Will return true any time the toggle value changes, NOT the toggle value itself!|

<div class='signature' markdown='1'>
```csharp
static bool Toggle(string text, Boolean& value, Vec2 size)
```
A toggleable button! A button will expand to fit the
text provided to it, vertically and horizontally. Text is re-used
as the id. Will return true any time the toggle value changes, NOT
the toggle value itself!
</div>

|  |  |
|--|--|
|string text|Text to display on the Toggle and id for             tracking element state. MUST be unique within current hierarchy.|
|Boolean& value|The current state of the toggle button! True              means it's toggled on, and false means it's toggled off.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) size|The layout size for this element in Hierarchy             space. If an axis is left as zero, it will be auto-calculated. For             X this is the remaining width of the current layout, and for Y this             is UI.LineHeight.|
|RETURNS: bool|Will return true any time the toggle value changes, NOT the toggle value itself!|





## Examples

### A toggle button

![A window with a toggle]({{site.screen_url}}/UI/ToggleWindow.jpg)

Toggle buttons swap between true and false when you press them! The
function requires a reference to a bool variable where the toggle's
state is stored. This allows you to manage the state yourself, and
it's completely valid for you to change the toggle state separately,
the UI element will update to match.

Note that `UI.Toggle` returns true _only_ when the toggle state has
changed, and does _not_ return the current state.

```csharp
Pose windowPoseToggle = new Pose(.3f, 0, 0, Quat.Identity);
bool toggleState      = true;
void ShowWindowToggle()
{
	UI.WindowBegin("Window Toggle", ref windowPoseToggle);

	if (UI.Toggle("Toggle me!", ref toggleState))
		Log.Info("Toggle just changed.");
	if (toggleState) UI.Label("Toggled On");
	else             UI.Label("Toggled Off");

	UI.WindowEnd();
}
```

