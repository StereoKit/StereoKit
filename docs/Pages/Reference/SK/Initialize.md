---
layout: default
title: SK.Initialize
description: Initializes StereoKit window, default resources, systems, etc.
---
# [SK]({{site.url}}/Pages/Reference/SK.html).Initialize

<div class='signature' markdown='1'>
```csharp
static bool Initialize(SKSettings settings)
```
Initializes StereoKit window, default resources, systems,
etc.
</div>

|  |  |
|--|--|
|[SKSettings]({{site.url}}/Pages/Reference/SKSettings.html) settings|The configuration settings for StereoKit.             This defines how StereoKit starts up and behaves, so it contains             things like app name, assets folder, display mode, etc. The              default settings are meant to be good for most, but you may want             to modify at least a few of these eventually!|
|RETURNS: bool|Returns true if all systems are successfully initialized!|




