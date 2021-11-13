---
layout: default
title: SK.Step
description: Steps all StereoKit systems, and inserts user code via callback between the appropriate system updates.
---
# [SK]({{site.url}}/Pages/Reference/SK.html).Step

<div class='signature' markdown='1'>
```csharp
static bool Step(Action onStep)
```
Steps all StereoKit systems, and inserts user code via
callback between the appropriate system updates.
</div>

|  |  |
|--|--|
|Action onStep|A callback where you put your application              code! This gets called between StereoKit systems, after frame              setup, but before render.|
|RETURNS: bool|If an exit message is received from the platform, this function will return false.|




