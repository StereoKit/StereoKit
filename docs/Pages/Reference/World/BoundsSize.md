---
layout: default
title: World.BoundsSize
description: This is the size of a rectangle within the play boundary/guardian's space, in meters if one exists. Check World.BoundsPose for the center point and orientation of the boundary, and check World.HasBounds to see if it exists at all!
---
# [World]({{site.url}}/Pages/Reference/World.html).BoundsSize

<div class='signature' markdown='1'>
static [Vec2]({{site.url}}/Pages/Reference/Vec2.html) BoundsSize{ get }
</div>

## Description
This is the size of a rectangle within the play
boundary/guardian's space, in meters if one exists. Check
`World.BoundsPose` for the center point and orientation of the
boundary, and check `World.HasBounds` to see if it exists at all!


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

