---
layout: default
title: StereoKitApp.Initialize
description: Initializes StereoKit window, default resources, systems, etc. Set settings before calling this function, if defaults need changed!
---
# [StereoKitApp]({{site.url}}/Pages/Reference/StereoKitApp.html).Initialize

<div class='signature' markdown='1'>
static bool Initialize(string name, [Runtime]({{site.url}}/Pages/Reference/Runtime.html) runtimePreference, bool fallback)
</div>

|  |  |
|--|--|
|string name|Name of the application, this shows up an the top of the Win32 window, and is submitted to OpenXR. OpenXR caps this at 128 characters.|
|[Runtime]({{site.url}}/Pages/Reference/Runtime.html) runtimePreference|Which runtime should we try to load?|
|bool fallback|If the preferred runtime fails, should we fall back to flatscreen?|
|RETURNS: bool|Returns true if all systems are successfully initialized!|

Initializes StereoKit window, default resources, systems, etc. Set settings before calling this function, if defaults need changed!



