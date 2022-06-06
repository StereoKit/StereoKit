---
layout: default
title: SKSettings
description: StereoKit initialization settings! Setup SK.settings with your data before calling SK.Initialize.
---
# struct SKSettings

StereoKit initialization settings! Setup SK.settings with
your data before calling SK.Initialize.

## Instance Fields and Properties

|  |  |
|--|--|
|string [appName]({{site.url}}/Pages/Reference/SKSettings/appName.html)|Name of the application, this shows up an the top of the Win32 window, and is submitted to OpenXR. OpenXR caps this at 128 characters.|
|string [assetsFolder]({{site.url}}/Pages/Reference/SKSettings/assetsFolder.html)|Where to look for assets when loading files! Final path will look like '[assetsFolder]/[file]', so a trailing '/' is unnecessary.|
|[DisplayBlend]({{site.url}}/Pages/Reference/DisplayBlend.html) [blendPreference]({{site.url}}/Pages/Reference/SKSettings/blendPreference.html)|What type of background blend mode do we prefer for this application? Are you trying to build an Opaque/Immersive/VR app, or would you like the display to be AnyTransparent, so the world will show up behind your content, if that's an option? Note that this is a preference only, and if it's not available on this device, the app will fall back to the runtime's preference instead! By default, (DisplayBlend.None) this uses the runtime's preference.|
|[DepthMode]({{site.url}}/Pages/Reference/DepthMode.html) [depthMode]({{site.url}}/Pages/Reference/SKSettings/depthMode.html)|What kind of depth buffer should StereoKit use? A fast one, a detailed one, one that uses stencils? By default, StereoKit uses a balanced mix depending on platform, prioritizing speed but opening up when there's headroom.|
|bool [disableFlatscreenMRSim]({{site.url}}/Pages/Reference/SKSettings/disableFlatscreenMRSim.html)|By default, StereoKit will simulate Mixed Reality input so developers can test MR spaces without being in a headset. If You don't want this, you can disable it with this setting!|
|bool [disableUnfocusedSleep]({{site.url}}/Pages/Reference/SKSettings/disableUnfocusedSleep.html)|By default, StereoKit will slow down when the application is out of focus. This is useful for saving processing power while the app is out-of-focus, but may not always be desired. In particular, running multiple copies of a SK app for testing networking code may benefit from this setting.|
|[DisplayMode]({{site.url}}/Pages/Reference/DisplayMode.html) [displayPreference]({{site.url}}/Pages/Reference/SKSettings/displayPreference.html)|Which display type should we try to load? Default is `DisplayMode.MixedReality`.|
|int [flatscreenHeight]({{site.url}}/Pages/Reference/SKSettings/flatscreenHeight.html)|If using Runtime.Flatscreen, the pixel size of the window on the screen.|
|int [flatscreenPosX]({{site.url}}/Pages/Reference/SKSettings/flatscreenPosX.html)|If using Runtime.Flatscreen, the pixel position of the window on the screen.|
|int [flatscreenPosY]({{site.url}}/Pages/Reference/SKSettings/flatscreenPosY.html)|If using Runtime.Flatscreen, the pixel position of the window on the screen.|
|int [flatscreenWidth]({{site.url}}/Pages/Reference/SKSettings/flatscreenWidth.html)|If using Runtime.Flatscreen, the pixel size of the window on the screen.|
|[LogLevel]({{site.url}}/Pages/Reference/LogLevel.html) [logFilter]({{site.url}}/Pages/Reference/SKSettings/logFilter.html)|The default log filtering level. This can be changed at runtime, but this allows you to set the log filter before Initialization occurs, so you can choose to get information from that. Default is LogLevel.Info.|
|bool [noFlatscreenFallback]({{site.url}}/Pages/Reference/SKSettings/noFlatscreenFallback.html)|If the preferred display fails, should we avoid falling back to flatscreen and just crash out? Default is false.|
|bool [overlayApp]({{site.url}}/Pages/Reference/SKSettings/overlayApp.html)|If the runtime supports it, should this application run as an overlay above existing applications? Check SK.System.overlayApp after initialization to see if the runtime could comply with this flag. This will always force StereoKit to work in a blend compositing mode.|
|uint [overlayPriority]({{site.url}}/Pages/Reference/SKSettings/overlayPriority.html)|For overlay applications, this is the order in which apps should be composited together. 0 means first, bottom of the stack, and uint.MaxValue is last, on top of the stack.|
