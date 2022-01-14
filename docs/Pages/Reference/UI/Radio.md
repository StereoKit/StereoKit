---
layout: default
title: UI.Radio
description: A Radio is similar to a button, except you can specify if it looks pressed or not regardless of interaction. This can be useful for radio-like behavior! Check an enum for a value, and use that as the 'active' state, Then switch to that enum value if Radio returns true.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).Radio

<div class='signature' markdown='1'>
```csharp
static bool Radio(string text, bool active)
```
A Radio is similar to a button, except you can specify if
it looks pressed or not regardless of interaction. This can be
useful for radio-like behavior! Check an enum for a value, and use
that as the 'active' state, Then switch to that enum value if Radio
returns true.
</div>

|  |  |
|--|--|
|string text|Text to display on the Radio and id for             tracking element state. MUST be unique within current hierarchy.|
|bool active|Does this button look like it's pressed?|
|RETURNS: bool|Will return true only on the first frame it is pressed!|





## Examples

### Radio button group

![A window with radio buttons]({{site.screen_url}}/UI/RadioWindow.jpg)

Radio buttons are a variety of Toggle button that behaves in a manner
more conducive to radio group style behavior. This is an example of
how to implement a small radio button group.

```csharp
Pose windowPoseRadio = new Pose(1.5f, 0, 0, Quat.Identity);
int  radioState      = 1;
void ShowWindowRadio()
{
	UI.WindowBegin("Window Radio", ref windowPoseRadio);

	if (UI.Radio("Option 1", radioState == 1)) radioState = 1;
	if (UI.Radio("Option 2", radioState == 2)) radioState = 2;
	if (UI.Radio("Option 3", radioState == 3)) radioState = 3;

	UI.WindowEnd();
}
```

