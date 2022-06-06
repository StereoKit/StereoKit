---
layout: default
title: UI.Text
description: Displays a large chunk of text on the current layout. This can include new lines and spaces, and will properly wrap once it fills the entire layout! Text uses the UI's current font settings, which can be changed with UI.Push/PopTextStyle.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).Text

<div class='signature' markdown='1'>
```csharp
static void Text(string text, TextAlign textAlign)
```
Displays a large chunk of text on the current layout.
This can include new lines and spaces, and will properly wrap
once it fills the entire layout! Text uses the UI's current font
settings, which can be changed with UI.Push/PopTextStyle.
</div>

|  |  |
|--|--|
|string text|The text you wish to display, there's no              additional parsing done to this text, so put it in as you want to             see it!|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) textAlign|Where should the text position itself             within its bounds? TextAlign.TopLeft is how most English text is             aligned.|





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

