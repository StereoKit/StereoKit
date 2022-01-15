---
layout: default
title: SKSettings.disableUnfocusedSleep
description: By default, StereoKit will slow down when the application is out of focus. This is useful for saving processing power while the app is out-of-focus, but may not always be desired. In particular, running multiple copies of a SK app for testing networking code may benefit from this setting.
---
# [SKSettings]({{site.url}}/Pages/Reference/SKSettings.html).disableUnfocusedSleep

<div class='signature' markdown='1'>
bool disableUnfocusedSleep{ get set }
</div>

## Description
By default, StereoKit will slow down when the
application is out of focus. This is useful for saving processing
power while the app is out-of-focus, but may not always be
desired. In particular, running multiple copies of a SK app for
testing networking code may benefit from this setting.

