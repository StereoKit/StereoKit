---
layout: default
title: UI.Text
description: Displays a large chunk of text on the current layout. This can include new lines and spaces, and will properly wrap once it fills the entire layout!
---
# [UI]({{site.url}}/Pages/Reference/UI.html).Text

<div class='signature' markdown='1'>
static void Text(string text, [TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) textAlign)
</div>

|  |  |
|--|--|
|string text|The text you wish to display, there's no              additional parsing done to this text, so put it in as you want to             see it!|
|[TextAlign]({{site.url}}/Pages/Reference/TextAlign.html) textAlign|Where should the text position itself             within its bounds? TextAlign.TopLeft is how most english text is             aligned.|

Displays a large chunk of text on the current layout.
This can include new lines and spaces, and will properly wrap
once it fills the entire layout!



