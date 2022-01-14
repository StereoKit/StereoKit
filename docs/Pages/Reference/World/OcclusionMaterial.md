---
layout: default
title: World.OcclusionMaterial
description: By default, this is a black(0,0,0,0) opaque unlit material that will occlude geometry, but won't show up as visible anywhere. You can override this with whatever material you would like.
---
# [World]({{site.url}}/Pages/Reference/World.html).OcclusionMaterial

<div class='signature' markdown='1'>
static [Material]({{site.url}}/Pages/Reference/Material.html) OcclusionMaterial{ get set }
</div>

## Description
By default, this is a black(0,0,0,0) opaque unlit
material that will occlude geometry, but won't show up as visible
anywhere. You can override this with whatever material you would
like.


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

