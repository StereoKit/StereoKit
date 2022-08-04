---
layout: default
title: World
description: World contains information about the real world around the user. This includes things like play boundaries, scene understanding, and other various things.
---
# static class World

World contains information about the real world around the
user. This includes things like play boundaries, scene understanding,
and other various things.

## Static Fields and Properties

|  |  |
|--|--|
|[Pose]({{site.url}}/Pages/Reference/Pose.html) [BoundsPose]({{site.url}}/Pages/Reference/World/BoundsPose.html)|This is the orientation and center point of the system's boundary/guardian. This can be useful to find the floor height! Not all systems have a boundary, so be sure to check `World.HasBounds` first.|
|[Vec2]({{site.url}}/Pages/Reference/Vec2.html) [BoundsSize]({{site.url}}/Pages/Reference/World/BoundsSize.html)|This is the size of a rectangle within the play boundary/guardian's space, in meters if one exists. Check `World.BoundsPose` for the center point and orientation of the boundary, and check `World.HasBounds` to see if it exists at all!|
|bool [HasBounds]({{site.url}}/Pages/Reference/World/HasBounds.html)|This refers to the play boundary, or guardian system that the system may have! Not all systems have this, so it's always a good idea to check this first!|
|bool [OcclusionEnabled]({{site.url}}/Pages/Reference/World/OcclusionEnabled.html)|Off by default. This tells StereoKit to load up and display an occlusion surface that allows the real world to occlude the application's digital content! Most systems may allow you to customize the visual appearance of this occlusion surface via the World.OcclusionMaterial. Check SK.System.worldOcclusionPresent to see if occlusion can be enabled. This will reset itself to false if occlusion isn't possible. Loading occlusion data is asynchronous, so occlusion may not occur immediately after setting this flag.|
|[Material]({{site.url}}/Pages/Reference/Material.html) [OcclusionMaterial]({{site.url}}/Pages/Reference/World/OcclusionMaterial.html)|By default, this is a black(0,0,0,0) opaque unlit material that will occlude geometry, but won't show up as visible anywhere. You can override this with whatever material you would like.|
|bool [RaycastEnabled]({{site.url}}/Pages/Reference/World/RaycastEnabled.html)|Off by default. This tells StereoKit to load up collision meshes for the environment, for use with World.Raycast. Check SK.System.worldRaycastPresent to see if raycasting can be enabled. This will reset itself to false if raycasting isn't possible. Loading raycasting data is asynchronous, so collision surfaces may not be available immediately after setting this flag.|
|float [RefreshInterval]({{site.url}}/Pages/Reference/World/RefreshInterval.html)|The refresh interval speed, in seconds. This is only applicable when using `WorldRefresh.Timer` for the refresh type. Note that the system may not be able to refresh as fast as you wish, and in that case, StereoKit will always refresh as soon as the previous refresh finishes.|
|float [RefreshRadius]({{site.url}}/Pages/Reference/World/RefreshRadius.html)|Radius, in meters, of the area that StereoKit should scan for world data. Default is 4. When using the `WorldRefresh.Area` refresh type, the world data will refresh when the user has traveled half this radius from the center of where the most recent refresh occurred.|
|[WorldRefresh]({{site.url}}/Pages/Reference/WorldRefresh.html) [RefreshType]({{site.url}}/Pages/Reference/World/RefreshType.html)|What information should StereoKit use to determine when the next world data refresh happens? See the `WorldRefresh` enum for details.|

## Static Methods

|  |  |
|--|--|
|[FromPerceptionAnchor]({{site.url}}/Pages/Reference/World/FromPerceptionAnchor.html)|Converts a Windows.Perception.Spatial.SpatialAnchor's pose into SteroKit's coordinate system. This can be great for interacting with some of the UWP spatial APIs such as WorldAnchors.  This method only works on UWP platforms, check SK.System.perceptionBridgePresent to see if this is available.|
|[FromSpatialNode]({{site.url}}/Pages/Reference/World/FromSpatialNode.html)|Converts a Windows Mirage spatial node GUID into a Pose based on its current position and rotation! Check SK.System.spatialBridgePresent to see if this is available to use. Currently only on HoloLens, good for use with the Windows QR code package.|
|[Raycast]({{site.url}}/Pages/Reference/World/Raycast.html)|World.RaycastEnabled must be set to true first! SK.System.worldRaycastPresent must also be true. This does a ray intersection with whatever represents the environment at the moment! In this case, it's a watertight collection of low resolution meshes calculated by the Scene Understanding extension, which is only provided by the Microsoft HoloLens runtime.|

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

