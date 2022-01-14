---
layout: default
title: UI.Button
description: A pressable button! A button will expand to fit the text provided to it, vertically and horizontally. Text is re-used as the id. Will return true only on the first frame it is pressed!
---
# [UI]({{site.url}}/Pages/Reference/UI.html).Button

<div class='signature' markdown='1'>
```csharp
static bool Button(string text)
```
A pressable button! A button will expand to fit the text
provided to it, vertically and horizontally. Text is re-used as the
id. Will return true only on the first frame it is pressed!
</div>

|  |  |
|--|--|
|string text|Text to display on the button and id for             tracking element state. MUST be unique within current hierarchy.|
|RETURNS: bool|Will return true only on the first frame it is pressed!|





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

