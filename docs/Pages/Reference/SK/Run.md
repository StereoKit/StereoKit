---
layout: default
title: SK.Run
description: This passes application execution over to StereoKit. This continuously steps all StereoKit systems, and inserts user code via callback between the appropriate system updates. Once execution completes, it properly calls the shutdown callback and shuts down StereoKit for you.  Using this method is important for compatibility with WASM and is the preferred method of controlling the main loop, over SK.Step.
---
# [SK]({{site.url}}/Pages/Reference/SK.html).Run

<div class='signature' markdown='1'>
```csharp
static void Run(Action onStep, Action onShutdown)
```
This passes application execution over to StereoKit.
This continuously steps all StereoKit systems, and inserts user
code via callback between the appropriate system updates. Once
execution completes, it properly calls the shutdown callback and
shuts down StereoKit for you.

Using this method is important for compatibility with WASM and is
the preferred method of controlling the main loop, over
`SK.Step`.
</div>

|  |  |
|--|--|
|Action onStep|A callback where you put your application              code! This gets called between StereoKit systems, after frame              setup, but before render.|
|Action onShutdown|A callback that gives you the             opportunity to shut things down while StereoKit is still active.             This is called after the last Step completes, and before             StereoKit shuts down.|




