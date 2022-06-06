---
layout: default
title: Bounds.Intersect
description: Calculate the intersection between a Ray, and these bounds. Returns false if no intersection occurred, and 'at' will contain the nearest intersection point to the start of the ray if an intersection is found!
---
# [Bounds]({{site.url}}/Pages/Reference/Bounds.html).Intersect

<div class='signature' markdown='1'>
```csharp
bool Intersect(Ray ray, Vec3& at)
```
Calculate the intersection between a Ray, and these
bounds. Returns false if no intersection occurred, and 'at' will
contain the nearest intersection point to the start of the ray if
an intersection is found!
</div>

|  |  |
|--|--|
|[Ray]({{site.url}}/Pages/Reference/Ray.html) ray|Any Ray in the same coordinate space as the             Bounds.|
|Vec3& at|If the return is true, this point will be the             closest intersection point to the origin of the Ray.|
|RETURNS: bool|True if an intersection occurred, false if not.|





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

