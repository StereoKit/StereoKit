---
layout: default
title: SK
description: This class contains functions for running the StereoKit library!
---
# static class SK

This class contains functions for running the StereoKit
library!




## Static Fields and Properties

|  |  |
|--|--|
|[DisplayMode]({{site.url}}/Pages/Reference/DisplayMode.html) [ActiveDisplayMode]({{site.url}}/Pages/Reference/SK/ActiveDisplayMode.html)|Since we can fallback to a different DisplayMode, this lets you check to see which Runtime was successfully initialized.|
|bool [IsInitialized]({{site.url}}/Pages/Reference/SK/IsInitialized.html)|Has StereoKit been successfully initialized already? If initialization was attempted and failed, this value will be false.|
|[SKSettings]({{site.url}}/Pages/Reference/SKSettings.html) [Settings]({{site.url}}/Pages/Reference/SK/Settings.html)|This is a copy of the settings that StereoKit was initialized with, so you can refer back to them a little easier. These are read only, and keep in mind that some settings are only requests! Check SK.System and other properties for the current state of StereoKit.|
|[SystemInfo]({{site.url}}/Pages/Reference/SystemInfo.html) [System]({{site.url}}/Pages/Reference/SK/System.html)|This structure contains information about the current system and its capabilites. There's a lot of different MR devices, so it's nice to have code for systems with particular characteristics!|
|UInt64 [VersionId]({{site.url}}/Pages/Reference/SK/VersionId.html)|An integer version Id! This is defined using a hex value with this format: `0xMMMMiiiiPPPPrrrr` in order of Major.mInor.Patch.pre-Release|
|string [VersionName]({{site.url}}/Pages/Reference/SK/VersionName.html)|Human-readable version name embedded in the StereoKitC DLL.|


## Static Methods

|  |  |
|--|--|
|[Initialize]({{site.url}}/Pages/Reference/SK/Initialize.html)|Initializes StereoKit window, default resources, systems, etc.|
|[PreLoadLibrary]({{site.url}}/Pages/Reference/SK/PreLoadLibrary.html)|If you need to call StereoKit code before calling SK.Initialize, you may need to explicitly load the library first. This can be useful for setting up a few things, but should probably be a pretty rare case.|
|[Quit]({{site.url}}/Pages/Reference/SK/Quit.html)|Lets StereoKit know it should quit! It'll finish the current frame, and after that Step will return that it wants to exit.|
|[Shutdown]({{site.url}}/Pages/Reference/SK/Shutdown.html)|Shuts down all StereoKit initialized systems. Release your own StereoKit created assets before calling this.|
|[Step]({{site.url}}/Pages/Reference/SK/Step.html)|Steps all StereoKit systems, and inserts user code via callback between the appropriate system updates.|

