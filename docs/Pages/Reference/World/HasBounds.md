---
layout: default
title: World.HasBounds
description: This refers to the play boundary, or guardian system that the system may have! Not all systems have this, so it's always a good idea to check this first!
---
# [World]({{site.url}}/Pages/Reference/World.html).HasBounds

<div class='signature' markdown='1'>
static bool HasBounds{ get }
</div>

## Description
This refers to the play boundary, or guardian system
that the system may have! Not all systems have this, so it's
always a good idea to check this first!


## Examples

```csharp
// Here's some quick and dirty lines for the play boundary rectangle!
if (World.HasBounds)
{
	Vec2   s    = World.BoundsSize/2;
	Matrix pose = World.BoundsPose.ToMatrix();
	Vec3   tl   = pose.Transform( new Vec3( s.x, 0,  s.y) );
	Vec3   br   = pose.Transform( new Vec3(-s.x, 0, -s.y) );
	Vec3   tr   = pose.Transform( new Vec3(-s.x, 0,  s.y) );
	Vec3   bl   = pose.Transform( new Vec3( s.x, 0, -s.y) );

	Lines.Add(tl, tr, Color.White, 1.5f*U.cm);
	Lines.Add(bl, br, Color.White, 1.5f*U.cm);
	Lines.Add(tl, bl, Color.White, 1.5f*U.cm);
	Lines.Add(tr, br, Color.White, 1.5f*U.cm);
}
```

