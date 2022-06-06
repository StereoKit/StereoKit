---
layout: default
title: Projection
description: The projection mode used by StereoKit for the main camera! You can use this with Renderer.Projection. These options are only available in flatscreen mode, as MR headsets provide very specific projection matrices.
---
# enum Projection

The projection mode used by StereoKit for the main camera! You
can use this with Renderer.Projection. These options are only
available in flatscreen mode, as MR headsets provide very
specific projection matrices.

## Enum Values

|  |  |
|--|--|
|Ortho|Orthographic projection mode is often used for tools, 2D rendering, thumbnails of 3D objects, or other similar cases. In this mode, parallel lines remain parallel regardless of how far they travel.|
|Perspective|This is the default projection mode, and the one you're most likely to be familiar with! This is where parallel lines will converge as they go into the distance.|

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

