---
layout: default
title: World.Raycast
description: World.RaycastEnabled must be set to true first! SK.System.worldRaycastPresent must also be true. This does a ray intersection with whatever represents the environment at the moment! In this case, it's a watertight collection of low resolution meshes calculated by the Scene Understanding extension, which is only provided by the Microsoft HoloLens runtime.
---
# [World]({{site.url}}/Pages/Reference/World.html).Raycast

<div class='signature' markdown='1'>
```csharp
static bool Raycast(Ray ray, Ray& intersection)
```
World.RaycastEnabled must be set to true first!
SK.System.worldRaycastPresent must also be true. This does a ray
intersection with whatever represents the environment at the
moment! In this case, it's a watertight collection of low
resolution meshes calculated by the Scene Understanding
extension, which is only provided by the Microsoft HoloLens
runtime.
</div>

|  |  |
|--|--|
|[Ray]({{site.url}}/Pages/Reference/Ray.html) ray|A world space ray that you'd like to try             intersecting with the world mesh.|
|Ray& intersection|The location of the intersection, and             direction of the world's surface at that point. This is only             valid if the method returns true.|
|RETURNS: bool|True if an intersection is detected, false if raycasting is disabled, or there was no intersection.|





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

