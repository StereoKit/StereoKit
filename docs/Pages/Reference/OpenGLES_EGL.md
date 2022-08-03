---
layout: default
title: OpenGLES_EGL
description: When using OpenGL ES with the EGL loader for rendering, this contains a number of variables that may be useful for doing advanced rendering tasks. This is the default rendering backend for Android, and Linux builds can be configured to use this with the SK_LINUX_EGL cmake option when building the core StereoKitC library.
---
# static class OpenGLES_EGL

When using OpenGL ES with the EGL loader for rendering,
this contains a number of variables that may be useful for doing
advanced rendering tasks. This is the default rendering backend for
Android, and Linux builds can be configured to use this with the
SK_LINUX_EGL cmake option when building the core StereoKitC
library.

## Static Fields and Properties

|  |  |
|--|--|
|IntPtr [Context]({{site.url}}/Pages/Reference/OpenGLES_EGL/Context.html)|This is the `EGLContext` StereoKit receives from `eglCreateContext`.|
|IntPtr [Display]({{site.url}}/Pages/Reference/OpenGLES_EGL/Display.html)|This is the `EGLDisplay` StereoKit receives from `eglGetDisplay`|
