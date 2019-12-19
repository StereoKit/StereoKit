---
layout: default
title: UI
description: This class is a collection of user interface and interaction methods! StereoKit uses an Immediate Mode gui system, which can be very easy to work with and modify during runtime.  You must call the UI method every frame you wish it to be available, and if you no longer want it to be present, you simply stop calling it! The id of the element is used to track its state from frame to frame, so for elements with state, you'll want to avoid changing the id during runtime! Ids are also scoped per-window, so different windows can re-use the same id, but a window cannot use the same id twice.
---
# UI

This class is a collection of user interface and interaction methods! StereoKit
uses an Immediate Mode gui system, which can be very easy to work with and modify during
runtime.

You must call the UI method every frame you wish it to be available, and if you no longer
want it to be present, you simply stop calling it! The id of the element is used to track
its state from frame to frame, so for elements with state, you'll want to avoid changing
the id during runtime! Ids are also scoped per-window, so different windows can re-use the
same id, but a window cannot use the same id twice.


## Fields and Properties

|  |  |
|--|--|
|float [LineHeight]({{site.url}}/Pages/Reference/UI/LineHeight.html)|This is the height of a single line of text with padding in the UI's layout system!|



## Methods

|  |  |
|--|--|
|[Button]({{site.url}}/Pages/Reference/UI/Button.html)|A pressable button! A button will expand to fit the text provided to it, vertically and horizontally. Text is re-used as the id. Will return true only on the first frame it is pressed!|
|[ButtonRound]({{site.url}}/Pages/Reference/UI/ButtonRound.html)|A pressable button! A button will expand to fit the text provided to it, vertically and horizontally. Text is re-used as the id. Will return true only on the first frame it is pressed!|
|[HSlider]({{site.url}}/Pages/Reference/UI/HSlider.html)|A horizontal slider element! You can stick your finger in it, and slide the value up and down.|
|[Label]({{site.url}}/Pages/Reference/UI/Label.html)|Adds some text to the layout! Text uses the UI's current font settings (which are currently not exposed). Can contain newlines! May have trouble with non-latin characters. Will advance layout to next line.|
|[PopId]({{site.url}}/Pages/Reference/UI/PopId.html)|Removes the last root id from the stack, and moves up to the one before it!|
|[PushId]({{site.url}}/Pages/Reference/UI/PushId.html)|Adds a root id to the stack for the following UI elements! This id is combined when hashing any following ids, to prevent id collisions in separate groups.|
|[SameLine]({{site.url}}/Pages/Reference/UI/SameLine.html)|Moves the current layout position back to the end of the line that just finished, so it can continue on the same line as the last element!|
|[Space]({{site.url}}/Pages/Reference/UI/Space.html)|Adds some space! If we're at the start of a new line, space is added vertically, otherwise, space is added horizontally.|
|[Toggle]({{site.url}}/Pages/Reference/UI/Toggle.html)|A toggleable button! A button will expand to fit the text provided to it, vertically and horizontally. Text is re-used as the id. Will return true any time the toggle value changes!|
|[WindowBegin]({{site.url}}/Pages/Reference/UI/WindowBegin.html)|Begins a new window! This will push a pose onto the transform stack, and all UI elements will be relative to that new pose. The pose is actually the top-center of the window. Must be finished with a call to UI.WindowEnd().|
|[WindowEnd]({{site.url}}/Pages/Reference/UI/WindowEnd.html)|Finishes a window! Must be called after UI.WindowBegin() and all elements have been drawn.|


