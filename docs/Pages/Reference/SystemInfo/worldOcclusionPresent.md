---
layout: default
title: SystemInfo.worldOcclusionPresent
description: Does this device support world occlusion of digital objects? If this is true, then World.OcclusionEnabled can be set to true, and World.OcclusionMaterial can be modified.
---
# [SystemInfo]({{site.url}}/Pages/Reference/SystemInfo.html).worldOcclusionPresent

<div class='signature' markdown='1'>
bool worldOcclusionPresent{ get }
</div>

## Description
Does this device support world occlusion of digital
objects? If this is true, then World.OcclusionEnabled can be set
to true, and World.OcclusionMaterial can be modified.


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
### Configuring Quality Occlusion

If you expect the user's environment to change a lot, or you
anticipate the user's environment may not be well scanned already,
then you may wish to boost the frequency of world data updates. By
default, StereoKit is quite conservative about scanning to reduce
computation, but this can be configured using the World.RefreshX
properties as seen here.

```csharp
// If occlusion is not available, the rest of the code will have no
// effect.
if (!SK.System.worldOcclusionPresent)
	Log.Info("Occlusion not available!");

// Configure SK to update the world data as fast as possible, this
// allows occlusion to accomodate better for moving objects.
World.OcclusionEnabled = true;
World.RefreshType     = WorldRefresh.Timer; // Refresh on a timer
World.RefreshInterval = 0; // Refresh every 0 seconds
World.RefreshRadius   = 6; // Get everything in a 6m radius
```

