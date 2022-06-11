---
layout: default
title: Mouse
description: This stores information about the mouse! What's its state, where's it pointed, do we even have one?
---
# struct Mouse

This stores information about the mouse! What's its state, where's
it pointed, do we even have one?

## Instance Fields and Properties

|  |  |
|--|--|
|bool [available]({{site.url}}/Pages/Reference/Mouse/available.html)|Is the mouse available to use? Most MR systems likely won't have a mouse!|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [pos]({{site.url}}/Pages/Reference/Mouse/pos.html)|Position of the mouse relative to the window it's in! This is the number of pixels from the top left corner of the screen.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [posChange]({{site.url}}/Pages/Reference/Mouse/posChange.html)|How much has the mouse's position changed in the current frame? Measured in pixels.|
|float [scroll]({{site.url}}/Pages/Reference/Mouse/scroll.html)|What's the current scroll value for the mouse's scroll wheel? TODO: Units|
|float [scrollChange]({{site.url}}/Pages/Reference/Mouse/scrollChange.html)|How much has the scroll wheel value changed during this frame? TODO: Units|
