---
layout: default
title: BackendGraphics
description: This describes the graphics API that StereoKit is using for rendering.
---
# enum BackendGraphics

This describes the graphics API that StereoKit is using for rendering.

## Enum Values

|  |  |
|--|--|
|D3D11|DirectX's Direct3D11 is used for rendering! This is used by default on Windows.|
|None|An invalid default value.|
|OpenGL_GLX|OpenGL is used for rendering, using GLX (OpenGL Extension to the X Window System) for loading. This is used by default on Linux.|
|OpenGL_WGL|OpenGL is used for rendering, using WGL (Windows Extensions to OpenGL) for loading. Native developers can configure SK to use this on Windows.|
|OpenGLES_EGL|OpenGL ES is used for rendering, using EGL (EGL Native Platform Graphics Interface) for loading. This is used by default on Android, and native developers can configure SK to use this on Linux.|
|WebGL|WebGL is used for rendering. This is used by default on Web.|
