---
layout: default
title: UI.Label
description: Adds some text to the layout! Text uses the UI's current font settings (which are currently not exposed). Can contain newlines! May have trouble with non-latin characters. Will advance layout to next line.
---
# [UI]({{site.url}}/Pages/Reference/UI.html).Label

<div class='signature' markdown='1'>
static void Label(string text, bool usePadding)
</div>

|  |  |
|--|--|
|string text|Label text to display. Can contain newlines!             May have trouble with non-latin characters. Doesn't use text as             id, so it can be non-unique.|
|bool usePadding|Should padding be included for             positioning this text? Sometimes you just want un-padded text!|

Adds some text to the layout! Text uses the UI's current
font settings (which are currently not exposed). Can contain
newlines! May have trouble with non-latin characters. Will
advance layout to next line.



