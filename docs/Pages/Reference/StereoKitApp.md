---
layout: default
title: StereoKitApp
description: This class contains functions for running the StereoKit library!
---
# StereoKitApp

This class contains functions for running the StereoKit library!




## Static Fields and Properties

|  |  |
|--|--|
|[Runtime]({{site.url}}/Pages/Reference/Runtime.html) [ActiveRuntime]({{site.url}}/Pages/Reference/StereoKitApp/ActiveRuntime.html)|Since we can fallback to a different Runtime, this lets you check to see which Runtime was successfully initialized.|
|bool [IsInitialized]({{site.url}}/Pages/Reference/StereoKitApp/IsInitialized.html)|Has StereoKit been successfully initialized already? If initialization was attempted and failed, this value will be false.|
|[Settings]({{site.url}}/Pages/Reference/Settings.html) [settings]({{site.url}}/Pages/Reference/StereoKitApp/settings.html)|Settings for more detailed initialization of StereoKit! Set these before calling Initialize, otherwise they'll be ignored.|
|[SystemInfo]({{site.url}}/Pages/Reference/SystemInfo.html) [System]({{site.url}}/Pages/Reference/StereoKitApp/System.html)|This structure contains information about the current system and its capabilites. There's a lot of different MR devices, so it's nice to have code for systems with particular characteristics!|
|UInt64 [VersionId]({{site.url}}/Pages/Reference/StereoKitApp/VersionId.html)|An integer version Id! This is defined using a hex value with this format:|
|string [VersionName]({{site.url}}/Pages/Reference/StereoKitApp/VersionName.html)|Human-readable version name embedded in the StereoKitC DLL.|


## Static Methods

|  |  |
|--|--|
|[Initialize]({{site.url}}/Pages/Reference/StereoKitApp/Initialize.html)|Initializes StereoKit window, default resources, systems, etc. Set settings before calling this function, if defaults need changed!|
|[Quit]({{site.url}}/Pages/Reference/StereoKitApp/Quit.html)|Lets StereoKit know it should quit! It'll finish the current frame, and after that Step will return that it wants to exit.|
|[Shutdown]({{site.url}}/Pages/Reference/StereoKitApp/Shutdown.html)|Shuts down all StereoKit initialized systems. Release your own StereoKit created assets before calling this.|
|[Step]({{site.url}}/Pages/Reference/StereoKitApp/Step.html)|Steps all StereoKit systems, and inserts user code via callback between the appropriate system updates.|

