---
layout: default
title: StereoKitApp.Step
description: Steps all StereoKit systems, and inserts user code via callback between the appropriate system updates.
---
# [StereoKitApp]({{site.url}}/Pages/Reference/StereoKitApp.html).Step

## Parameters

|  |  |
|--|--|
|onStep|A callback where you put your application code! This gets called between StereoKit systems, after frame setup, but before render.|

## Returns
If an exit message is received from the platform, this function will return false.

## Description
Steps all StereoKit systems, and inserts user code via callback between the appropriate system updates.

