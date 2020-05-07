---
layout: default
title: UI.WindowBegin
description: Begins a new window! This will push a pose onto the transform stack, and all UI elements will be relative to that new pose. The pose is actually the top-center of the window. Must be finished with a call to UI.WindowEnd().
---
# [UI]({{site.url}}/Pages/Reference/UI.html).WindowBegin

<div class='signature' markdown='1'>
static void WindowBegin(string text, Pose& pose, [Vec2]({{site.url}}/Pages/Reference/Vec2.html) size, [UIWin]({{site.url}}/Pages/Reference/UIWin.html) windowType, [UIMove]({{site.url}}/Pages/Reference/UIMove.html) moveType)
</div>

|  |  |
|--|--|
|string text|Text to display on the window title, should be              unique as it will be used as the window's id.|
|Pose& pose|The pose state for the window! If showHeader              is true, the user will be able to grab this header and move it              around.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) size|Physical size of the window! Should be set to              a non-zero value, otherwise it'll default to 32mm. If y is zero,              it'll expand to contain all elements within it.|
|[UIWin]({{site.url}}/Pages/Reference/UIWin.html) windowType|Describes how the window should be drawn,             use a header, a body, neither, or both?|
|[UIMove]({{site.url}}/Pages/Reference/UIMove.html) moveType|Describes how the window will move when              dragged around.|

Begins a new window! This will push a pose onto the
transform stack, and all UI elements will be relative to that new
pose. The pose is actually the top-center of the window. Must be
finished with a call to UI.WindowEnd().



