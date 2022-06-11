---
layout: default
title: UI.HSlider
description: A horizontal slider element! You can stick your finger in it, and slide the value up and down.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).HSlider

<div class='signature' markdown='1'>
```csharp
static bool HSlider(string id, Single& value, float min, float max, float step, float width, UIConfirm confirmMethod)
```
A horizontal slider element! You can stick your finger
in it, and slide the value up and down.
</div>

|  |  |
|--|--|
|string id|An id for tracking element state. MUST be unique             within current hierarchy.|
|Single& value|The value that the slider will store slider              state in.|
|float min|The minimum value the slider can set, left side              of the slider.|
|float max|The maximum value the slider can set, right              side of the slider.|
|float step|Locks the value to intervals of step. Starts              at min, and increments by step.|
|float width|Physical width of the slider on the window. 0             will fill the remaining amount of window space.|
|[UIConfirm]({{site.url}}/Pages/Reference/UIConfirm.html) confirmMethod|How should the slider be activated?             Push will be a push-button the user must press first, and pinch             will be a tab that the user must pinch and drag around.|
|RETURNS: bool|Returns true any time the value changes.|

<div class='signature' markdown='1'>
```csharp
static bool HSlider(string id, Double& value, double min, double max, double step, float width, UIConfirm confirmMethod)
```
A horizontal slider element! You can stick your finger
in it, and slide the value up and down.
</div>

|  |  |
|--|--|
|string id|An id for tracking element state. MUST be unique             within current hierarchy.|
|Double& value|The value that the slider will store slider              state in.|
|double min|The minimum value the slider can set, left side              of the slider.|
|double max|The maximum value the slider can set, right              side of the slider.|
|double step|Locks the value to intervals of step. Starts              at min, and increments by step.|
|float width|Physical width of the slider on the window. 0             will fill the remaining amount of window space.|
|[UIConfirm]({{site.url}}/Pages/Reference/UIConfirm.html) confirmMethod|How should the slider be activated?             Push will be a push-button the user must press first, and pinch             will be a tab that the user must pinch and drag around.|
|RETURNS: bool|Returns true any time the value changes.|





## Examples

### Horizontal Sliders

![A window with a slider]({{site.screen_url}}/UI/SliderWindow.jpg)

A slider will slide between two values at increments. The function
requires a reference to a float variable where the slider's state is
stored. This allows you to manage the state yourself, and it's
completely valid for you to change the slider state separately, the
UI element will update to match.

Note that `UI.HSlider` returns true _only_ when the slider state has
changed, and does _not_ return the current state.

```csharp
Pose  windowPoseSlider = new Pose(.9f, 0, 0, Quat.Identity);
float sliderState      = 0.5f;
void ShowWindowSlider()
{
	UI.WindowBegin("Window Slider", ref windowPoseSlider);

	if (UI.HSlider("Slider", ref sliderState, 0, 1, 0.1f))
		Log.Info($"Slider value just changed: {sliderState}");

	UI.WindowEnd();
}
```

