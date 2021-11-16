---
layout: default
title: UI.WindowBegin
description: Begins a new window! This will push a pose onto the transform stack, and all UI elements will be relative to that new pose. The pose is actually the top-center of the window. Must be finished with a call to UI.WindowEnd().
---
# [UI]({{site.url}}/Pages/Reference/UI.html).WindowBegin

<div class='signature' markdown='1'>
```csharp
static void WindowBegin(string text, Pose& pose, Vec2 size, UIWin windowType, UIMove moveType)
```
Begins a new window! This will push a pose onto the
transform stack, and all UI elements will be relative to that new
pose. The pose is actually the top-center of the window. Must be
finished with a call to UI.WindowEnd().
</div>

|  |  |
|--|--|
|string text|Text to display on the window title and id for             tracking element state. MUST be unique within current hierarchy.|
|Pose& pose|The pose state for the window! If showHeader              is true, the user will be able to grab this header and move it              around.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) size|Physical size of the window! If either              dimension is 0, then the size on that axis will be auto-             calculated based on the content provided during the previous              frame.|
|[UIWin]({{site.url}}/Pages/Reference/UIWin.html) windowType|Describes how the window should be drawn,             use a header, a body, neither, or both?|
|[UIMove]({{site.url}}/Pages/Reference/UIMove.html) moveType|Describes how the window will move when              dragged around.|

<div class='signature' markdown='1'>
```csharp
static void WindowBegin(string text, Pose& pose, UIWin windowType, UIMove moveType)
```
Begins a new window! This will push a pose onto the
transform stack, and all UI elements will be relative to that new
pose. The pose is actually the top-center of the window. Must be
finished with a call to UI.WindowEnd(). This override omits the
size value, so the size will be auto-calculated based on the
content provided during the previous frame.
</div>

|  |  |
|--|--|
|string text|Text to display on the window title and id for             tracking element state. MUST be unique within current hierarchy.|
|Pose& pose|The pose state for the window! If showHeader              is true, the user will be able to grab this header and move it              around.|
|[UIWin]({{site.url}}/Pages/Reference/UIWin.html) windowType|Describes how the window should be drawn,             use a header, a body, neither, or both?|
|[UIMove]({{site.url}}/Pages/Reference/UIMove.html) moveType|Describes how the window will move when              dragged around.|





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

