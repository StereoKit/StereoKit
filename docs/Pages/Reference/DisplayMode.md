---
layout: default
title: DisplayMode
description: Specifies a type of display mode StereoKit uses, like Mixed Reality headset display vs. a PC display, or even just rendering to an offscreen surface, or not rendering at all!
---
# enum DisplayMode

Specifies a type of display mode StereoKit uses, like
Mixed Reality headset display vs. a PC display, or even just
rendering to an offscreen surface, or not rendering at all!

## Enum Values

|  |  |
|--|--|
|Flatscreen|Creates a flat, Win32 window, and simulates some MR functionality. Great for debugging.|
|MixedReality|Creates an OpenXR instance, and drives display/input through that.|
|None|Not tested yet, but this is meant to run StereoKit without rendering to any display at all. This would allow for rendering to textures, running a server that can do MR related tasks, etc.|
