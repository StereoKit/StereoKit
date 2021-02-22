---
layout: default
title: DisplayMode
description: Specifies a type of display mode StereoKit uses, like Mixed Reality headset display vs. a PC display, or even just rendering to an offscreen surface, or not rendering at all!
---
# DisplayMode

Specifies a type of display mode StereoKit uses, like
Mixed Reality headset display vs. a PC display, or even just
rendering to an offscreen surface, or not rendering at all!




## Static Fields and Properties

|  |  |
|--|--|
|[DisplayMode]({{site.url}}/Pages/Reference/DisplayMode.html) [Flatscreen]({{site.url}}/Pages/Reference/DisplayMode/Flatscreen.html)|Creates a flat, Win32 window, and simulates some MR functionality. Great for debugging.|
|[DisplayMode]({{site.url}}/Pages/Reference/DisplayMode.html) [MixedReality]({{site.url}}/Pages/Reference/DisplayMode/MixedReality.html)|Creates an OpenXR instance, and drives display/input through that.|
|[DisplayMode]({{site.url}}/Pages/Reference/DisplayMode.html) [None]({{site.url}}/Pages/Reference/DisplayMode/None.html)|Not tested yet, but this is meant to run StereoKit without rendering to any display at all. This would allow for rendering to textures, running a server that can do MR related tasks, etc.|


