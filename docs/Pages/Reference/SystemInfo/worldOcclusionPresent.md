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

