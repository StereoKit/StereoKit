---
layout: default
title: WorldRefresh
description: A settings flag that lets you describe the behavior of how StereoKit will refresh data about the world mesh, if applicable. This is used with World.RefreshType.
---
# enum WorldRefresh

A settings flag that lets you describe the behavior of how
StereoKit will refresh data about the world mesh, if applicable. This
is used with `World.RefreshType`.

## Enum Values

|  |  |
|--|--|
|Area|Refreshing occurs when the user leaves the area that was most recently scanned. This area is a sphere that is 0.5 of the World.RefreshRadius.|
|Timer|Refreshing happens at timer intervals. If an update doesn't happen in time, the next update will happen as soon as possible. The timer interval is configurable via `World.RefreshInterval`.|

## Examples

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

