---
layout: default
title: SK.ExecuteOnMain
description: This will queue up some code to be run on StereoKit's main thread! Immediately after StereoKit's Step, all callbacks registered here will execute, and then removed from the list.
---
# [SK]({{site.url}}/Pages/Reference/SK.html).ExecuteOnMain

<div class='signature' markdown='1'>
```csharp
static void ExecuteOnMain(Action action)
```
This will queue up some code to be run on StereoKit's main
thread! Immediately after StereoKit's Step, all callbacks
registered here will execute, and then removed from the list.
</div>

|  |  |
|--|--|
|Action action|Some code to run! This Action will persist in             a list until after Step, at which point it is removed and dropped.|




