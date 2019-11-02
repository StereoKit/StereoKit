
---
layout: default
title: StereoKitApp.Initialize
description: Initializes StereoKit window, default resources, systems, etc. Set settings before calling this function, if defaults need changed!
---
# [StereoKitApp](/assets/pages/Reference/StereoKitApp.md).Initialize

## Parameters
|  |  |
|--|--|
|name|Name of the application, this shows up an the top of the Win32 window, and is submitted to OpenXR. OpenXR caps this at 128 characters.|
|runtimePreference|Which runtime should we try to load?|
|fallback|If the preferred runtime fails, should we fall back to flatscreen?|

## Returns
Returns true if all systems are successfully initialized!

## Description
Initializes StereoKit window, default resources, systems, etc. Set settings before calling this function, if defaults need changed!
