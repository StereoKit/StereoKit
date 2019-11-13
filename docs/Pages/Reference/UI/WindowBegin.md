---
layout: default
title: UI.WindowBegin
description: Begins a new window! This will push a pose onto the transform stack, and all UI elements will be relative to that new pose. The pose is actually the top-center of the window. Must be finished with a call to UI.WindowEnd().
---
# [UI]({{site.url}}/Pages/Reference/UI.html).WindowBegin

<div class='signature' markdown='1'>
static void WindowBegin(string text, Pose& pose, Vec2 size, bool showHeader)
</div>

|  |  |
|--|--|
|string text|Text to display on the window title, should be unique as it              will be used as the window's id.|
|Pose& pose|The pose state for the window! If showHeader is true, the user             will be able to grab this header and move it around.|
|Vec2 size|Physical size of the window! Should be set to a non-zero value, otherwise             it'll default to 32mm. If y is zero, it'll expand to contain all elements within it.|
|bool showHeader|Should the window show a header bar? Header bar includes a title,             and is grabbable when it's visible.|

Begins a new window! This will push a pose onto the transform stack, and all UI
elements will be relative to that new pose. The pose is actually the top-center
of the window. Must be finished with a call to UI.WindowEnd().



