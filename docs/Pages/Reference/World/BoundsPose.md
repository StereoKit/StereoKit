---
layout: default
title: World.BoundsPose
description: This is the orientation and center point of the system's boundary/guardian. This can be useful to find the floor height! Not all systems have a boundary, so be sure to check World.HasBounds first.
---
# [World]({{site.url}}/Pages/Reference/World.html).BoundsPose

<div class='signature' markdown='1'>
static [Pose]({{site.url}}/Pages/Reference/Pose.html) BoundsPose{ get }
</div>

## Description
This is the orientation and center point of the system's
boundary/guardian. This can be useful to find the floor height!
Not all systems have a boundary, so be sure to check
`World.HasBounds` first.


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

