---
layout: default
title: Renderer.OverrideCaptureFilter
description: The CaptureFilter is a layer mask for Mixed Reality Capture, or 2nd person observer rendering. On HoloLens and WMR, this is the video rendering feature. This allows you to hide, or reveal certain draw calls when rendering video output.  By default, the CaptureFilter will always be the same as Render.LayerFilter, overriding this will mean this filter no longer updates with LayerFilter.
---
# [Renderer]({{site.url}}/Pages/Reference/Renderer.html).OverrideCaptureFilter

<div class='signature' markdown='1'>
```csharp
static void OverrideCaptureFilter(bool useOverrideFilter, RenderLayer overrideFilter)
```
The CaptureFilter is a layer mask for Mixed Reality
Capture, or 2nd person observer rendering. On HoloLens and WMR,
this is the video rendering feature. This allows you to hide, or
reveal certain draw calls when rendering video output.

By default, the CaptureFilter will always be the same as
`Render.LayerFilter`, overriding this will mean this filter no
longer updates with `LayerFilter`.
</div>

|  |  |
|--|--|
|bool useOverrideFilter|Enables (true) or disables (false)             the overridden filter value provided here.|
|RenderLayer overrideFilter|The filter for capture rendering to             use. This is ignored if useOverrideFilter is false.|




