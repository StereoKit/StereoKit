---
layout: default
title: UI.Input
description: This is an input field where users can input text to the app! Selecting it will spawn a virtual keyboard, or act as the keyboard focus. Hitting escape or enter, or focusing another UI element will remove focus from this Input.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).Input

<div class='signature' markdown='1'>
```csharp
static bool Input(string id, String& value, Vec2 size, TextContext type)
```
This is an input field where users can input text to the
app! Selecting it will spawn a virtual keyboard, or act as the
keyboard focus. Hitting escape or enter, or focusing another UI
element will remove focus from this Input.
</div>

|  |  |
|--|--|
|string id|An id for tracking element state. MUST be unique             within current hierarchy.|
|String& value|The string that will store the Input's              content in.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) size|Size of the Input in Hierarchy local meters.             Zero axes will auto-size.|
|[TextContext]({{site.url}}/Pages/Reference/TextContext.html) type|What category of text this Input represents.             This may affect what kind of soft keyboard will be displayed, if             one is shown to the user.|
|RETURNS: bool|Returns true every time the contents of 'value' change.|





## Examples

### Text Input

![A window with a text input]({{site.screen_url}}/UI/InputWindow.jpg)

The `UI.Input` element allows users to enter text. Upon selecting the
element, a virtual keyboard will appear on platforms that provide
one.  The function requires a reference to a string variable where
the input's state is stored. This allows you to manage the state
yourself, and it's completely valid for you to change the input state
separately, the UI element will update to match.

`UI.Input` will return true on frames where the text has _just_
changed.

```csharp
Pose   windowPoseInput = new Pose(1.2f, 0, 0, Quat.Identity);
string inputState      = "Initial text";
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

