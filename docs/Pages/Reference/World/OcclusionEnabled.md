---
layout: default
title: World.OcclusionEnabled
description: Off by default. This tells StereoKit to load up and display an occlusion surface that allows the real world to occlude the application's digital content! Most systems may allow you to customize the visual appearance of this occlusion surface via the World.OcclusionMaterial. Check SK.System.worldOcclusionPresent to see if occlusion can be enabled. This will reset itself to false if occlusion isn't possible. Loading occlusion data is asynchronous, so occlusion may not occur immediately after setting this flag.
---
# [World]({{site.url}}/Pages/Reference/World.html).OcclusionEnabled

<div class='signature' markdown='1'>
static bool OcclusionEnabled{ get set }
</div>

## Description
Off by default. This tells StereoKit to load up and
display an occlusion surface that allows the real world to
occlude the application's digital content! Most systems may allow
you to customize the visual appearance of this occlusion surface
via the World.OcclusionMaterial.
Check SK.System.worldOcclusionPresent to see if occlusion can be
enabled. This will reset itself to false if occlusion isn't
possible. Loading occlusion data is asynchronous, so occlusion
may not occur immediately after setting this flag.


## Examples

### Basic World Occlusion

A simple example of turning on the occlusion mesh and overriding the
default material so it's visible. For normal usage where you just
want to let the real world occlude geometry, the only important
element is to just set `World.OcclusionEnabled = true;`.
```csharp
Material occlusionMatPrev;

public void Start()
{
	if (!SK.System.worldOcclusionPresent)
		Log.Info("Occlusion not available!");

	// If not available, this will have no effect
	World.OcclusionEnabled = true;

	// Override the default occluding material
	occlusionMatPrev = World.OcclusionMaterial;
	World.OcclusionMaterial = Material.Default;
}

public void Stop()
{
	// Restore the previous occlusion material
	World.OcclusionMaterial = occlusionMatPrev;

	// Stop occlusion
	World.OcclusionEnabled = false;
}
```

