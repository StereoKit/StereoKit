---
layout: default
title: UI.LastElementFocused
description: Tells the Focused state of the most recent UI element that used an id.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).LastElementFocused

<div class='signature' markdown='1'>
static [BtnState]({{site.url}}/Pages/Reference/BtnState.html) LastElementFocused{ get }
</div>

## Description
Tells the Focused state of the most recent UI element that
used an id.


## Examples

### Checking UI element status
It can sometimes be nice to know how the user is interacting with a
particular UI element! The UI.LastElementX functions can be used to
query a bit of this information, but only for _the most recent_ UI
element that **uses an id**!

![A window containing the status of a UI element]({{site.screen_url}}/UI/LastElementAPI.jpg)

So in this example, we're querying the information for the "Slider"
UI element. Note that UI.Text does NOT use an id, which is why this
works.
```csharp
UI.WindowBegin("Last Element API", ref windowPose);

UI.HSlider("Slider", ref sliderVal, 0, 1, 0.1f, 0, UIConfirm.Pinch);
UI.Text("Element Info:", TextAlign.TopCenter);
if (UI.LastElementHandUsed(Handed.Left ).IsActive()) UI.Label("Left");
if (UI.LastElementHandUsed(Handed.Right).IsActive()) UI.Label("Right");
if (UI.LastElementFocused               .IsActive()) UI.Label("Focused");
if (UI.LastElementActive                .IsActive()) UI.Label("Active");

UI.WindowEnd();
```

