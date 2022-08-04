---
layout: default
title: Renderer.Projection
description: For flatscreen applications only! This allows you to change the camera projection between perspective and orthographic projection. This may be of interest for some category of UI work, but is generally a niche piece of functionality.  Swapping between perspective and orthographic will also switch the clipping planes and field of view to the values associated with that mode. See SetClip/SetFov for perspective, and SetOrthoClip/SetOrthoSize for orthographic.
---
# [Renderer]({{site.url}}/Pages/Reference/Renderer.html).Projection

<div class='signature' markdown='1'>
static [Projection]({{site.url}}/Pages/Reference/Projection.html) Projection{ get set }
</div>

## Description
For flatscreen applications only! This allows you to
change the camera projection between perspective and orthographic
projection. This may be of interest for some category of UI work,
but is generally a niche piece of functionality.

Swapping between perspective and orthographic will also switch the
clipping planes and field of view to the values associated with
that mode. See `SetClip`/`SetFov` for perspective, and
`SetOrthoClip`/`SetOrthoSize` for orthographic.


## Examples

### Toggling the projection mode
Only in flatscreen apps, there is the option to change the main
camera's projection mode between perspective and orthographic.
```csharp
if (SK.ActiveDisplayMode == DisplayMode.Flatscreen &&
	Input.Key(Key.P).IsJustActive())
{
	Renderer.Projection = Renderer.Projection == Projection.Perspective
		? Projection.Ortho
		: Projection.Perspective;
}
```

