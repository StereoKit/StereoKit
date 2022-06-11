---
layout: default
title: Bounds.FromCorner
description: Create a bounding box from a corner, plus box dimensions.
---
# [Bounds]({{site.url}}/Pages/Reference/Bounds.html).FromCorner

<div class='signature' markdown='1'>
```csharp
static Bounds FromCorner(Vec3 bottomLeftBack, Vec3 dimensions)
```
Create a bounding box from a corner, plus box dimensions.
</div>

|  |  |
|--|--|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) bottomLeftBack|The -X,-Y,-Z corner of the box.|
|[Vec3]({{site.url}}/Pages/Reference/Vec3.html) dimensions|The total dimensions of the box.|
|RETURNS: [Bounds]({{site.url}}/Pages/Reference/Bounds.html)|A Bounds object that extends from bottomLeftBack to bottomLeftBack+dimensions.|





## Examples

### General Usage

```csharp
// All these create bounds for a 1x1x1m cube around the origin!
Bounds bounds = new Bounds(Vec3.One);
bounds = Bounds.FromCorner(new Vec3(-0.5f, -0.5f, -0.5f), Vec3.One);
bounds = Bounds.FromCorners(
	new Vec3(-0.5f, -0.5f, -0.5f),
	new Vec3( 0.5f,  0.5f,  0.5f));

// Note that positions must be in a coordinate space relative to 
// the bounds!
if (bounds.Contains(new Vec3(0,0.25f,0)))
	Log.Info("Super easy to check if something's in it!");

// Casting a ray at a bounds is trivial too, again, ensure 
// coordinates are in the same space!
Ray ray = new Ray(Vec3.Up, -Vec3.Up);
if (bounds.Intersect(ray, out Vec3 at))
	Log.Info("Bounds intersection at " + at); // <0, 0.5f, 0>

// You can also scale a Bounds using the '*' operator overload, 
// this is really useful if you're working with the Bounds of a
// Model that you've scaled. It will scale the center as well as
// the size!
bounds = bounds * 0.5f;

// Scale the current bounds reference using 'Scale'
bounds.Scale(0.5f);

// Scale the bounds by a Vec3
bounds = bounds * new Vec3(1, 10, 0.5f);
```

