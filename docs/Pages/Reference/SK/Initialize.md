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

<div class='signature' markdown='1'>
```csharp
static bool Initialize(string projectName, string assetsFolder)
```
This is a _very_ rudimentary way to initialize StereoKit,
it doesn't take much, but a robust application will prefer to use
an overload that takes SKSettings. This uses all the default
settings, which are primarily configured for development.
</div>

|  |  |
|--|--|
|string projectName|Name of the application, this shows up an             the top of the Win32 window, and is submitted to OpenXR. OpenXR             caps this at 128 characters.|
|string assetsFolder|Where to look for assets when loading             files! Final path will look like '[assetsFolder]/[file]', so a             trailing '/' is unnecessary.|
|RETURNS: bool|Returns true if all systems are successfully initialized!|




