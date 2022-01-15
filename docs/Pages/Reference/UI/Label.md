---
layout: default
title: UI.Label
description: Adds some text to the layout! Text uses the UI's current font settings, which can be changed with UI.Push/PopTextStyle. Can contain newlines!
---
# [UI]({{site.url}}/Pages/Reference/UI.html).Label

<div class='signature' markdown='1'>
```csharp
static void Label(string text, bool usePadding)
```
Adds some text to the layout! Text uses the UI's current
font settings, which can be changed with UI.Push/PopTextStyle. Can
contain newlines!
</div>

|  |  |
|--|--|
|string text|Label text to display. Can contain newlines!             Doesn't use text as id, so it can be non-unique.|
|bool usePadding|Should padding be included for             positioning this text? Sometimes you just want un-padded text!|

<div class='signature' markdown='1'>
```csharp
static void Label(string text, Vec2 size)
```
Adds some text to the layout, but this overload allows you
can specify the size that you want it to use. Text uses the UI's
current font settings, which can be changed with
UI.Push/PopTextStyle. Can contain newlines!
</div>

|  |  |
|--|--|
|string text|Label text to display. Can contain newlines!             Doesn't use text as id, so it can be non-unique.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) size|The layout size for this element in Hierarchy             space. If an axis is left as zero, it will be auto-calculated. For             X this is the remaining width of the current layout, and for Y this             is UI.LineHeight.|





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

