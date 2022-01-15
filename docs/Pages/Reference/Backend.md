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
|[BackendXRType]({{site.url}}/Pages/Reference/BackendXRType.html) [XRType]({{site.url}}/Pages/Reference/Backend/XRType.html)|What technology is being used to drive StereoKit's XR functionality? OpenXR is the most likely candidate here, but if you're running the flatscreen Simulator, or running in the web with WebXR, then this will reflect that.|


