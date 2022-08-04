---
layout: default
title: World.RaycastEnabled
description: Off by default. This tells StereoKit to load up collision meshes for the environment, for use with World.Raycast. Check SK.System.worldRaycastPresent to see if raycasting can be enabled. This will reset itself to false if raycasting isn't possible. Loading raycasting data is asynchronous, so collision surfaces may not be available immediately after setting this flag.
---
# [World]({{site.url}}/Pages/Reference/World.html).RaycastEnabled

<div class='signature' markdown='1'>
static bool RaycastEnabled{ get set }
</div>

## Description
Off by default. This tells StereoKit to load up
collision meshes for the environment, for use with World.Raycast.
Check SK.System.worldRaycastPresent to see if raycasting can be
enabled. This will reset itself to false if raycasting isn't
possible. Loading raycasting data is asynchronous, so collision
surfaces may not be available immediately after setting this
flag.


## Examples

### Basic World Raycasting

World.RaycastEnabled must be true before calling World.Raycast, or
you won't ever intersect with any world geometry.
```csharp
public void Start()
{
	if (!SK.System.worldRaycastPresent)
		Log.Info("World raycasting not available!");

	// This must be enabled before calling World.Raycast
	World.RaycastEnabled = true;
}

public void Stop() => World.RaycastEnabled = false;

public void Step()
{
	// Raycast out the index finger of each hand, and draw a red sphere
	// at the intersection point.
	for (int i = 0; i < 2; i++)
	{
		Hand hand = Input.Hand(i);
		if (!hand.IsTracked) continue;

		Ray fingerRay = hand[FingerId.Index, JointId.Tip].Pose.Ray;
		if (World.Raycast(fingerRay, out Ray at))
			Mesh.Sphere.Draw(Material.Default, Matrix.TS(at.position, 0.03f), new Color(1, 0, 0));
	}
}
```

