---
layout: default
title: Backend
description: This class exposes some of StereoKit's backend functionality. This allows for tighter integration with certain platforms, but also means your code becomes less portable. Everything in this class should be guarded by availability checks.
---
# static class Backend

This class exposes some of StereoKit's backend functionality.
This allows for tighter integration with certain platforms, but also
means your code becomes less portable. Everything in this class should
be guarded by availability checks.

## Static Fields and Properties

|  |  |
|--|--|
|[BackendGraphics]({{site.url}}/Pages/Reference/BackendGraphics.html) [Graphics]({{site.url}}/Pages/Reference/Backend/Graphics.html)|This describes the graphics API thatStereoKit is using for rendering. StereoKit uses D3D11 for Windows platforms, and a flavor of OpenGL for Linux, Android, and Web.|
|[BackendPlatform]({{site.url}}/Pages/Reference/BackendPlatform.html) [Platform]({{site.url}}/Pages/Reference/Backend/Platform.html)|What kind of platform is StereoKit running on? This can be important to tell you what APIs or functionality is available to the app.|
|[BackendXRType]({{site.url}}/Pages/Reference/BackendXRType.html) [XRType]({{site.url}}/Pages/Reference/Backend/XRType.html)|What technology is being used to drive StereoKit's XR functionality? OpenXR is the most likely candidate here, but if you're running the flatscreen Simulator, or running in the web with WebXR, then this will reflect that.|
