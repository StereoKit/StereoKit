---
layout: default
title: UI.Text
description: Displays a large chunk of text on the current layout. This can include new lines and spaces, and will properly wrap once it fills the entire layout!
---
# [UI]({{site.url}}/Pages/Reference/UI.html).Text

<div class='signature' markdown='1'>
```csharp
static void Text(string text, TextAlign textAlign)
```
Displays a large chunk of text on the current layout.
This can include new lines and spaces, and will properly wrap
once it fills the entire layout!
</div>

|  |  |
|--|--|
|string text|The text you wish to display, there's no              additional parsing done to this text, so put it in as you want to             see it!|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) textAlign|Where should the text position itself             within its bounds? TextAlign.TopLeft is how most english text is             aligned.|





## Examples

### Separating UI Visually

![A window with text and a separator]({{site.screen_url}}/UI/SeparatorWindow.jpg)

Toggle buttons swap between true and false when you press them! The
function requires a reference to a bool variable where the toggle's
state is stored. This allows you to manage the state yourself, and
it's completely valid for you to change the toggle state separately,
the UI element will update to match.

Note that `UI.Toggle` returns true _only_ when the toggle state has
changed, and does _not_ return the current state.

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

