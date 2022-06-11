---
layout: default
title: SK.SetWindow
description: Android only. This is for telling StereoKit about the active Android window surface. In particular, Xamarin's ISurfaceHolderCallback2 gets SurfaceCreated and SurfaceDestroyed events, and these events should feed into this function.
---
# [SK]({{site.url}}/Pages/Reference/SK.html).SetWindow

<div class='signature' markdown='1'>
```csharp
static void SetWindow(IntPtr window)
```
Android only. This is for telling StereoKit about the
active Android window surface. In particular, Xamarin's
ISurfaceHolderCallback2 gets SurfaceCreated and SurfaceDestroyed
events, and these events should feed into this function.
</div>

|  |  |
|--|--|
|IntPtr window|This is an ISurfaceHolder.Surface.Handle or             equivalent pointer.|




