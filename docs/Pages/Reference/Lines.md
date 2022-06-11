---
layout: default
title: Lines
description: A line drawing class! This is an easy way to visualize lines or relationships between objects. The current implementation uses a quad strip that always faces the user, via vertex shader manipulation.
---
# static class Lines

A line drawing class! This is an easy way to visualize lines
or relationships between objects. The current implementation uses a
quad strip that always faces the user, via vertex shader
manipulation.

## Static Methods

|  |  |
|--|--|
|[Add]({{site.url}}/Pages/Reference/Lines/Add.html)|Adds a line to the environment for the current frame.|
|[AddAxis]({{site.url}}/Pages/Reference/Lines/AddAxis.html)|Displays an RGB/XYZ axis widget at the pose! Each line is extended along the positive direction of each axis, so the red line is +X, green is +Y, and blue is +Z. A white line is drawn along -Z to indicate the Forward vector of the pose (-Z is forward in StereoKit).|
