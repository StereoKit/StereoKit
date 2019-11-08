---
layout: default
title: StereoKitApp.Step
description: Steps all StereoKit systems, and inserts user code via callback between the appropriate system updates.
---
# [StereoKitApp]({{site.url}}/Pages/Reference/StereoKitApp.html).Step
<div class='signature' markdown='1'>
static bool Step(Action onStep)
</div>
Steps all StereoKit systems, and inserts user code via callback between the appropriate system updates.

|  |  |
|--|--|
|Action onStep|A callback where you put your application code! This gets called between StereoKit systems, after frame setup, but before render.|
|RETURNS: bool|If an exit message is received from the platform, this function will return false.|



